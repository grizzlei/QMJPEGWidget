### Qt MJPEG Streamer widget [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
_Hasan Karaman, karaman.hasan@yahoo.com, 2022_

MJPEGStreamer is a easy to use MJPEG surface for Qt GUI applications. Written using Qt5 but can be easily used with Qt4 with minor tweaks.
It uses pure `QTcpSocket`, and no external libraries are needed. Currently supports basic and digest authentication. 

Example use (dynamic creation):
```c++
MJPEGStreamer * stream = new MJPEGStreamer(this);
stream->set_url("http://88.53.197.250/axis-cgi/mjpg/video.cgi?resolution=320x240");
stream->start();
stream->stop();
```

Or if you prefer to use Qt Designer, drag and drop a `Widget` to somewhere on the window then simply promote it to a `MJPEGStreamer`.

_Disclaimer: address in the example is a public MJPEG streaming server and it is not guaranteed to remain the way it is. Use it at your own risk._
