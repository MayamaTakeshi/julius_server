var net = require('net');
var fs = require('fs');

var HOST = '192.168.225.131';

var client1 = new net.Socket();

client1.on('error', function (e) {
  console.log("client1 ERROR: "+e.code);
});

client1.on('data', function(data) {
    console.log(`client1 data: ${data}`)  
});

client1.on('close', function() {
    console.log(`client1 closed`);
});

client1.connect(10500, HOST, function() {
  console.log('client1 connected to server: ' + HOST + ':' + 10500);

  // raw file must be big endian encoded as signed-integer
  var wav_file = "../artifacts/ohayou_gozaimasu.4times.raw"

  const file = fs.createReadStream(wav_file)

  file.pipe(client1)
});





