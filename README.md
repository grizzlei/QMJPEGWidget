### Qt MJPEG Streamer widget [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
_Hasan Karaman, karaman.hasan@yahoo.com, 2022_

MJPEGStreamer is a very simple to use MJPEG surface for Qt5 (and Qt4 but with minor tweaks).

Example use:
```c++
MJPEGStreamer * stream = new MJPEGStreamer();
stream->set_url("http://88.53.197.250/axis-cgi/mjpg/video.cgi?resolution=320x240");
stream->start();
stream->stop();
```
_Disclaimer: address in the example is a public MJPEG streaming server and it is not guaranteed to remain the way it is. Use it at your own risk._
