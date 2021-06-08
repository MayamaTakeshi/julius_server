var net = require('net');
var fs = require('fs');
var wav = require('wav');

var HOST = '192.168.225.131';
PORT = 5530

var wav_file = "../artifacts/ohayou_gozaimasu.4times.wav"

var acc = []

var buff = null

function setup() {

  var client = new net.Socket();

  client.on('error', function (e) {
    console.log("client ERROR: "+e.code);
  });

  client.on('data', function(data) {
      console.log(`client data: ${data}`)  
  });

  client.on('close', function() {
    console.log(`client closed`);
  });

  client.connect(5530, HOST, function() {
      console.log('client connected to server: ' + HOST + ':' + 5530);

      const file = fs.createReadStream(wav_file)
      const reader = new wav.Reader()

      reader.on('format', function (format) {
        //console.log(format)
      })

      reader.on('data', function (data) {
        console.log("reader data")
        acc.push(data)
      })

      reader.on('error', error => {
        console.log(`reader error {error}`)
      })

      reader.on('end', () => {
        console.log(`reader end`)
        buff = Buffer.concat(acc)

        const SIZE = 16000
        var tid = setInterval(() => {
          var data = buff.slice(0, SIZE)
          if(data.length == 0) {
            clearInterval(tid) 
            return
          }
          buff = buff.slice(SIZE)
          console.log(`Sending ${data.length}`)
          client.write(data) 
        }, 20)
      })

      file.pipe(reader)
  })
}


setup()

