### julius_server

## Overview

This is a forking server for julius speech recognition engine.
It waits on a TCP port for clients and forks processes to handle them.

Obs:
  - the speech recognition results are reported on a simplified format (we don't use the XML format julius use with option -module)
  - it is not possible to send commands to julius during operation


## Installation

You should install julius and acoustic models.
In case you want to test with Japanese and if you are using Debian 10 and is root, this should work:
```
./install_julius_and_japanese_dictation_kit.sh
```

Then build the app by doing (you might need to adjust the Makefile with the location of julius if you build it from source)
```
make
```

## Details
We originally implemented a plugin to permit to connect using a single socket to the server:
```
$ ./julius_server -C /usr/local/src/git/dictation-kit/main.jconf -C /usr/local/src/git/dictation-kit/am-gmm.jconf -plugindir . -input fullduplex -cutsilence -nostrip 

```
This seemed to work OK:
```
$ node test.js -b 320 ../artifacts/ohayou_gozaimasu.4times.raw
2021-06-10 14:48:07.206 : client connected to server: localhost:10500
2021-06-10 14:48:07.213 : msg=+READY
2021-06-10 14:48:07.215 : msg=+SPEECH_READY
2021-06-10 14:48:08.109 : msg=+SPEECH_START
2021-06-10 14:48:11.012 : msg=+SPEECH_STOP
2021-06-10 14:48:11.132 : msg=+RESULT:おはようございます。
2021-06-10 14:48:11.133 : msg=+SPEECH_READY
2021-06-10 14:48:16.295 : msg=+SPEECH_START
2021-06-10 14:48:19.210 : msg=+SPEECH_STOP
2021-06-10 14:48:19.311 : msg=+RESULT:おはようございます。
2021-06-10 14:48:19.312 : msg=+SPEECH_READY
2021-06-10 14:48:24.384 : msg=+SPEECH_START
2021-06-10 14:48:27.412 : msg=+SPEECH_STOP
2021-06-10 14:48:27.515 : msg=+RESULT:おはようございます。
2021-06-10 14:48:27.516 : msg=+SPEECH_READY
2021-06-10 14:48:32.612 : msg=+SPEECH_START
2021-06-10 14:48:35.636 : msg=+SPEECH_STOP
2021-06-10 14:48:35.756 : msg=+RESULT:おはようございます。
2021-06-10 14:48:35.757 : msg=+SPEECH_READY
2021-06-10 14:48:39.954 : No more data from file.
```

However, when trying sending large packets, it failed:

```
$ node test.js -b 10000 ../artifacts/ohayou_gozaimasu.4times.raw 
2021-06-10 14:50:00.136 : client connected to server: localhost:10500
2021-06-10 14:50:00.145 : msg=+READY
2021-06-10 14:50:00.147 : msg=+SPEECH_READY
2021-06-10 14:50:00.191 : msg=+SPEECH_START
2021-06-10 14:50:00.300 : msg=+SPEECH_STOP
2021-06-10 14:50:00.484 : msg=+SPEECH_START
2021-06-10 14:50:00.532 : msg=+SPEECH_STOP
2021-06-10 14:50:00.724 : msg=+SPEECH_START
2021-06-10 14:50:00.816 : msg=+SPEECH_STOP
2021-06-10 14:50:00.975 : msg=+SPEECH_START
2021-06-10 14:50:01.080 : msg=+SPEECH_STOP
2021-06-10 14:50:01.219 : No more data from file.
2021-06-10 14:50:01.439 : msg=+RESULT:おはようございます。
2021-06-10 14:50:01.440 : msg=+SPEECH_READY
```

So we added support of adinnet:
```
$ ./julius_server -C /usr/local/src/git/dictation-kit/main.jconf -C /usr/local/src/git/dictation-kit/am-gmm.jconf -input adinnet -adport 0 -cutsilence -nostrip
```

And this way, there was no problem:
```
$ node test.js -b 320 ../artifacts/ohayou_gozaimasu.4times.raw 
2021-06-10 14:50:42.987 : client connected to server: localhost:10500
2021-06-10 14:50:42.994 : msg=+AUDIO_PORT:43395
2021-06-10 14:50:42.995 : audio_port=43395
2021-06-10 14:50:42.997 : msg=+READY
2021-06-10 14:50:42.999 : msg=+SPEECH_READY
2021-06-10 14:50:42.999 : audio_port_client connected to server: localhost:43395
2021-06-10 14:50:43.891 : msg=+SPEECH_START
2021-06-10 14:50:46.803 : msg=+SPEECH_STOP
2021-06-10 14:50:46.875 : msg=+RESULT:おはようございます。
2021-06-10 14:50:46.878 : msg=+SPEECH_READY
2021-06-10 14:50:52.091 : msg=+SPEECH_START
2021-06-10 14:50:54.996 : msg=+SPEECH_STOP
2021-06-10 14:50:55.054 : msg=+RESULT:おはようございます。
2021-06-10 14:50:55.058 : msg=+SPEECH_READY
2021-06-10 14:51:00.167 : msg=+SPEECH_START
2021-06-10 14:51:03.196 : msg=+SPEECH_STOP
2021-06-10 14:51:03.260 : msg=+RESULT:おはようございます。
2021-06-10 14:51:03.262 : msg=+SPEECH_READY
2021-06-10 14:51:08.396 : msg=+SPEECH_START
2021-06-10 14:51:11.419 : msg=+SPEECH_STOP
2021-06-10 14:51:11.496 : msg=+RESULT:おはようございます。
2021-06-10 14:51:11.499 : msg=+SPEECH_READY
2021-06-10 14:51:15.737 : No more data from file.


$ node test.js -b 10000 ../artifacts/ohayou_gozaimasu.4times.raw 
2021-06-10 14:51:47.647 : client connected to server: localhost:10500
2021-06-10 14:51:47.654 : msg=+AUDIO_PORT:42711
2021-06-10 14:51:47.654 : audio_port=42711
2021-06-10 14:51:47.656 : msg=+READY
2021-06-10 14:51:47.658 : msg=+SPEECH_READY
2021-06-10 14:51:47.659 : audio_port_client connected to server: localhost:42711
2021-06-10 14:51:47.747 : msg=+SPEECH_START
2021-06-10 14:51:48.726 : No more data from file.
2021-06-10 14:51:48.779 : msg=+SPEECH_STOP
2021-06-10 14:51:48.848 : msg=+RESULT:おはようございます。
2021-06-10 14:51:48.851 : msg=+SPEECH_READY
2021-06-10 14:51:49.028 : msg=+SPEECH_START
2021-06-10 14:51:50.018 : msg=+SPEECH_STOP
2021-06-10 14:51:50.068 : msg=+RESULT:おはようございます。
2021-06-10 14:51:50.070 : msg=+SPEECH_READY
2021-06-10 14:51:50.488 : msg=+SPEECH_START
2021-06-10 14:51:51.490 : msg=+SPEECH_STOP
2021-06-10 14:51:51.552 : msg=+RESULT:おはようございます。
2021-06-10 14:51:51.555 : msg=+SPEECH_READY
2021-06-10 14:51:51.974 : msg=+SPEECH_START
2021-06-10 14:51:52.966 : msg=+SPEECH_STOP
2021-06-10 14:51:53.033 : msg=+RESULT:おはようございます。
2021-06-10 14:51:53.035 : msg=+SPEECH_READY


$ node test.js -b 50000 ../artifacts/ohayou_gozaimasu.4times.raw 
2021-06-10 14:52:28.578 : client connected to server: localhost:10500
2021-06-10 14:52:28.584 : msg=+AUDIO_PORT:37663
2021-06-10 14:52:28.585 : audio_port=37663
2021-06-10 14:52:28.586 : msg=+READY
2021-06-10 14:52:28.587 : msg=+SPEECH_READY
2021-06-10 14:52:28.588 : audio_port_client connected to server: localhost:37663
2021-06-10 14:52:28.671 : msg=+SPEECH_START
2021-06-10 14:52:28.830 : No more data from file.
2021-06-10 14:52:29.687 : msg=+SPEECH_STOP
2021-06-10 14:52:29.756 : msg=+RESULT:おはようございます。
2021-06-10 14:52:29.759 : msg=+SPEECH_READY
2021-06-10 14:52:30.191 : msg=+SPEECH_START
2021-06-10 14:52:31.165 : msg=+SPEECH_STOP
2021-06-10 14:52:31.225 : msg=+RESULT:おはようございます。
2021-06-10 14:52:31.228 : msg=+SPEECH_READY
2021-06-10 14:52:31.647 : msg=+SPEECH_START
2021-06-10 14:52:32.636 : msg=+SPEECH_STOP
2021-06-10 14:52:32.713 : msg=+RESULT:おはようございます。
2021-06-10 14:52:32.716 : msg=+SPEECH_READY
2021-06-10 14:52:33.139 : msg=+SPEECH_START
2021-06-10 14:52:34.152 : msg=+SPEECH_STOP
2021-06-10 14:52:34.223 : msg=+RESULT:おはようございます。
2021-06-10 14:52:34.226 : msg=+SPEECH_READY
```

## Concurrent Session Limit
You can control how many speech recognition session will be allowed by setting env var MAX_CONCURRENT_SESSIONS.
Ex:
```
MAX_CONCURRENT_SESSIONS=5 ./julius_server -C /usr/local/src/git/dictation-kit/main.jconf -C /usr/local/src/git//dictation-kit/am-gmm.jconf -input adinnet -adport 0 -cutsilence -nostrip
```
