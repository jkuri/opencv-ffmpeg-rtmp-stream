#include <iostream>
#include <vector>

#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

void stream_video(double width, double height, int fps, int camID)
{
  av_register_all();
  avformat_network_init();

  const char *output = "rtmp://localhost/show/stream";
  const AVRational dst_fps = {fps, 1};
  int ret;

  // initialize video capture device
  cv::VideoCapture cam(camID);
  if (!cam.isOpened())
  {
    std::cout << "Failed to open video capture device!" << std::endl;
    exit(1);
  }

  cam.set(cv::CAP_PROP_FRAME_WIDTH, width);
  cam.set(cv::CAP_PROP_FRAME_HEIGHT, height);

  // allocate cv::Mat with extra bytes (required by AVFrame::data)
  std::vector<uint8_t> imgbuf(height * width * 3 + 16);
  cv::Mat image(height, width, CV_8UC3, imgbuf.data(), width * 3);

  // open output format context
  AVFormatContext *outctx = nullptr;
  ret = avformat_alloc_output_context2(&outctx, nullptr, "flv", output);
  if (ret < 0)
  {
    std::cout << "Could not allocate output format context!" << std::endl;
    exit(1);
  }

  // open output IO context
  if (!(outctx->oformat->flags & AVFMT_NOFILE))
  {
    ret = avio_open2(&outctx->pb, output, AVIO_FLAG_WRITE, nullptr, nullptr);
    if (ret < 0)
    {
      std::cout << "Could not open output IO context!" << std::endl;
      exit(1);
    }
  }

  // create new video stream
  AVCodec *codec = avcodec_find_encoder(outctx->oformat->video_codec);
  AVStream *strm = avformat_new_stream(outctx, codec);
  avcodec_get_context_defaults3(strm->codec, codec);
  strm->codec->width = width;
  strm->codec->height = height;
  strm->codec->pix_fmt = codec->pix_fmts[0];
  strm->codec->time_base = strm->time_base = av_inv_q(dst_fps);
  strm->r_frame_rate = strm->avg_frame_rate = dst_fps;
  if (outctx->oformat->flags & AVFMT_GLOBALHEADER)
  {
    strm->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }

  AVDictionary *opts = nullptr;
  av_dict_set(&opts, "preset", "superfast", 0);
  av_dict_set(&opts, "tune", "zerolatency", 0);

  // open video encoder
  ret = avcodec_open2(strm->codec, codec, &opts);
  if (ret < 0)
  {
    std::cout << "Could not open video encoder!" << std::endl;
    exit(1);
  }

  // initialize sample scaler
  SwsContext *swsctx = sws_getCachedContext(nullptr, width, height, AV_PIX_FMT_BGR24, width, height, strm->codec->pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
  if (!swsctx)
  {
    std::cout << "Could not initialize sample scaler!" << std::endl;
    exit(1);
  }

  // allocate frame buffer for encoding
  AVFrame *frame = av_frame_alloc();
  std::vector<uint8_t> framebuf(avpicture_get_size(strm->codec->pix_fmt, width, height));
  avpicture_fill(reinterpret_cast<AVPicture *>(frame), framebuf.data(), strm->codec->pix_fmt, width, height);
  frame->width = width;
  frame->height = height;
  frame->format = static_cast<int>(strm->codec->pix_fmt);

  // write header
  ret = avformat_write_header(outctx, nullptr);
  if (ret < 0)
  {
    std::cout << "Could not write header!" << std::endl;
    exit(1);
  }

  // encoding loop
  int64_t frame_pts = 0;
  unsigned nb_frames = 0;
  bool end_of_stream = false;
  int got_pkt = 0;

  do
  {
    if (!end_of_stream)
    {
      cam >> image;
      // convert cv::Mat to AVFrame.
      const int stride[] = {static_cast<int>(image.step[0])};
      sws_scale(swsctx, &image.data, stride, 0, image.rows, frame->data, frame->linesize);
      frame->pts = frame_pts++;
    }
    // encode video frame.
    AVPacket pkt;
    pkt.data = nullptr;
    pkt.size = 0;
    av_init_packet(&pkt);
    ret = avcodec_encode_video2(strm->codec, &pkt, end_of_stream ? nullptr : frame, &got_pkt);
    if (ret < 0)
    {
      std::cout << "Error encoding video!" << std::endl;
      exit(1);
    }

    if (got_pkt)
    {
      // rescale packet timestamp.
      pkt.duration = 1;
      av_packet_rescale_ts(&pkt, strm->codec->time_base, strm->time_base);
      // write packet.
      av_write_frame(outctx, &pkt);

      // av_interleaved_write_frame(outctx, &pkt);

      std::cout << " Frames: " << nb_frames << '\r' << std::flush;
      ++nb_frames;
    }

    av_free_packet(&pkt);
  } while (!end_of_stream || got_pkt);

  av_write_trailer(outctx);
  std::cout << nb_frames << " frames encoded" << std::endl;

  av_frame_free(&frame);
  avcodec_close(strm->codec);
  avio_close(outctx->pb);
  avformat_free_context(outctx);
}

int main()
{
  double width = 640, height = 480, fps = 25;
  int camID = 0;

  stream_video(width, height, fps, camID);

  return 0;
}
