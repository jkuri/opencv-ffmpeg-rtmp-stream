#include <iostream>
#include <vector>

#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

cv::VideoCapture get_device(int camID, double width, double height)
{
  cv::VideoCapture cam(camID);
  if (!cam.isOpened())
  {
    std::cout << "Failed to open video capture device!" << std::endl;
    exit(1);
  }

  cam.set(cv::CAP_PROP_FRAME_WIDTH, width);
  cam.set(cv::CAP_PROP_FRAME_HEIGHT, height);

  return cam;
}

void initialize_avformat_context(AVFormatContext *&fctx, const char *format_name)
{
  int ret = avformat_alloc_output_context2(&fctx, nullptr, format_name, nullptr);
  if (ret < 0)
  {
    std::cout << "Could not allocate output format context!" << std::endl;
    exit(1);
  }
}

void initialize_io_context(AVFormatContext *&fctx, const char *output)
{
  if (!(fctx->oformat->flags & AVFMT_NOFILE))
  {
    int ret = avio_open2(&fctx->pb, output, AVIO_FLAG_WRITE, nullptr, nullptr);
    if (ret < 0)
    {
      std::cout << "Could not open output IO context!" << std::endl;
      exit(1);
    }
  }
}

void set_codec_params(AVFormatContext *&fctx, AVCodecContext *&codec_ctx, double width, double height, int fps)
{
  const AVRational dst_fps = {fps, 1};

  codec_ctx->codec_tag = 0;
  codec_ctx->bit_rate = 400000;
  codec_ctx->width = width;
  codec_ctx->height = height;
  codec_ctx->time_base.den = fps;
  codec_ctx->time_base.num = 1;
  codec_ctx->gop_size = 12;
  codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
  codec_ctx->framerate = dst_fps;
  codec_ctx->time_base = av_inv_q(dst_fps);
  if (fctx->oformat->flags & AVFMT_GLOBALHEADER)
  {
    codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }
}

void initialize_codec_stream(AVStream *&stream, AVCodecContext *&codec_ctx, AVCodec *&codec)
{
  int ret = avcodec_parameters_from_context(stream->codecpar, codec_ctx);
  if (ret < 0)
  {
    std::cout << "Could not initialize stream codec parameters!" << std::endl;
    exit(1);
  }

  AVDictionary *codec_options = nullptr;
  av_dict_set(&codec_options, "profile", "high", 0);
  av_dict_set(&codec_options, "preset", "superfast", 0);
  av_dict_set(&codec_options, "tune", "zerolatency", 0);

  // open video encoder
  ret = avcodec_open2(codec_ctx, codec, &codec_options);
  if (ret < 0)
  {
    std::cout << "Could not open video encoder!" << std::endl;
    exit(1);
  }
}

SwsContext *initialize_sample_scaler(AVCodecContext *codec_ctx, double width, double height)
{
  SwsContext *swsctx = sws_getContext(width, height, AV_PIX_FMT_BGR24, width, height, codec_ctx->pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
  if (!swsctx)
  {
    std::cout << "Could not initialize sample scaler!" << std::endl;
    exit(1);
  }

  return swsctx;
}

AVFrame *allocate_frame_buffer(AVCodecContext *codec_ctx, double width, double height)
{
  AVFrame *frame = av_frame_alloc();

  std::vector<uint8_t> framebuf(av_image_get_buffer_size(codec_ctx->pix_fmt, width, height, 1));
  av_image_fill_arrays(frame->data, frame->linesize, framebuf.data(), codec_ctx->pix_fmt, width, height, 1);
  frame->width = width;
  frame->height = height;
  frame->format = static_cast<int>(codec_ctx->pix_fmt);

  return frame;
}

void stream_video(double width, double height, int fps, int camID)
{
  av_register_all();
  avformat_network_init();

  const char *output = "rtmp://localhost/live/stream";
  int ret;
  auto cam = get_device(camID, width, height);
  std::vector<uint8_t> imgbuf(height * width * 3 + 16);
  cv::Mat image(height, width, CV_8UC3, imgbuf.data(), width * 3);
  AVFormatContext *ofmt_ctx = nullptr;
  AVCodec *out_codec = nullptr;
  AVStream *out_stream = nullptr;
  AVCodecContext *out_codec_ctx = nullptr;

  initialize_avformat_context(ofmt_ctx, "flv");
  initialize_io_context(ofmt_ctx, output);

  out_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  out_stream = avformat_new_stream(ofmt_ctx, out_codec);
  out_codec_ctx = avcodec_alloc_context3(out_codec);

  set_codec_params(ofmt_ctx, out_codec_ctx, width, height, fps);
  initialize_codec_stream(out_stream, out_codec_ctx, out_codec);

  auto *swsctx = initialize_sample_scaler(out_codec_ctx, width, height);
  auto *frame = allocate_frame_buffer(out_codec_ctx, width, height);

  ret = avformat_write_header(ofmt_ctx, nullptr);
  if (ret < 0)
  {
    std::cout << "Could not write header!" << std::endl;
    exit(1);
  }

  // encoding loop
  int64_t frame_pts = 0;
  unsigned nb_frames = 0;
  bool end_of_stream = false;

  do
  {
    nb_frames++;

    if (!end_of_stream)
    {
      cam >> image;
      const int stride[] = {static_cast<int>(image.step[0])};
      sws_scale(swsctx, &image.data, stride, 0, image.rows, frame->data, frame->linesize);
      frame->pts = frame_pts++;
    }

    AVPacket pkt = { 0 };
    av_init_packet(&pkt);

    ret = avcodec_send_frame(out_codec_ctx, frame);
    if (ret < 0)
    {
      std::cout << "Error sending frame to codec context!" << std::endl;
      exit(1);
    }

    ret = avcodec_receive_packet(out_codec_ctx, &pkt);
    if (ret < 0)
    {
      std::cout << "Error receiving packet from codec context!" << std::endl;
      exit(1);
    }

    av_packet_rescale_ts(&pkt, out_codec_ctx->time_base, out_stream->time_base);
    pkt.pos = -1;
    av_interleaved_write_frame(ofmt_ctx, &pkt);

    av_packet_unref(&pkt);
  } while (!end_of_stream);

  av_write_trailer(ofmt_ctx);
  std::cout << nb_frames << " frames encoded" << std::endl;

  av_frame_free(&frame);
  avcodec_close(out_codec_ctx);
  avio_close(ofmt_ctx->pb);
  avformat_free_context(ofmt_ctx);
}

int main()
{
  av_log_set_level(AV_LOG_DEBUG);
  double width = 640, height = 480;
  int camID = 1, fps = 30;

  stream_video(width, height, fps, camID);

  return 0;
}
