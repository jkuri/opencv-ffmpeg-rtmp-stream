# OpenCV FFMpeg RTMP Stream

Example of live video streaming over RTMP protocol using OpenCV and FFMpeg.

### Installation Guide

#### Install FFMpeg

On MacOS.

```sh
brew install ffmpeg --with-sdl2 --with-freetype
```

On Ubuntu Linux.

```sh
sudo apt-get install ffmpeg libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavresample-dev libavdevice-dev -y
```

#### Install OpenCV

For installing OpenCV there's a script inside `scripts/` folder:

```sh
./scripts/install-opencv.sh
```

#### Build Nginx RTMP Server Docker image

```sh
docker build -t nginx-rtmp ./nginx-rtmp-server
```

### Compile & run

To compile source code just run:

```sh
mkdir -p build && cd build
cmake .. && make
```

Then run RTMP server using Docker image you built before and use attached `nginx.conf` configuration file.

```sh
docker run -p 1935:1935 -dit -v /path-to-this-repository/nginx-rtmp-config:/etc/nginx --name nginx-rtmp nginx-rtmp
```

Check that server is working with `docker ps` command:

```sh
CONTAINER ID        IMAGE               COMMAND                  CREATED             STATUS              PORTS                                          NAMES
ec976b5ecd93        nginx-rtmp          "/bin/sh -c 'nginx -…"   21 minutes ago      Up 21 minutes       0.0.0.0:1935->1935/tcp                         nginx-rtmp
```

Run the program to start streaming:

```sh
./build/rtmp-stream
```

To set up different options for stream, here is `./rtmp-stream -h` output

```sh
SYNOPSIS
        ./rtmp-stream [-c <camera>] [-o <output>] [-f <fps>] [-w <width>] [-h <height>] [-b <bitrate>] [-p <profile>] [-l <log>]

OPTIONS
        -c, --camera <camera>
                    camera ID (default: 0)

        -o, --output <output>
                    output RTMP server (default: rtmp://localhost/live/stream)

        -f, --fps <fps>
                    frames-per-second (default: 30)

        -w, --width <width>
                    video width (default: 800)

        -h, --height <height>
                    video height (default: 640)

        -b, --bitrate <bitrate>
                    stream bitrate in kb/s (default: 300000)

        -p, --profile <profile>
                    H264 codec profile (baseline | high | high10 | high422 | high444 | main) (default: high444)

        -l, --log <log>
                    print debug output (default: false)
```

Use VLC or `ffplay` to connect to live video stream:

```sh
ffplay -sync ext rtmp://localhost/live/stream
```

If everything worked you should see live-stream video.

<p align="center">
  <img src="https://user-images.githubusercontent.com/1796022/38590611-907bb7ba-3d32-11e8-994c-d3efe5b3651d.png">
</p>

### LICENCE

MIT
