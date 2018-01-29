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
sudo apt-get install ffmpeg -y
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

Use VLC or `ffplay` to connect to live video stream:

```sh
ffplay -sync ext rtmp://localhost/show/stream
```

If everything worked you should see live-stream video.

<p align="center">
  <img src="https://user-images.githubusercontent.com/1796022/35540393-fe658900-0555-11e8-8129-d5bdebc3a3d9.png">
</p>

### LICENCE

MIT
