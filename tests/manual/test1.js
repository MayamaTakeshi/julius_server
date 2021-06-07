var net = require('net');
var fs = require('fs');

var HOST = '192.168.225.131';

var client = new net.Socket();

client.on('end', function() {
  console.log("client end")
})

client.on('error', function (e) {
  console.log(`client error: ${e.code}`);
})

client.on('data', function(data) {
  console.log(`client data: ${data}`)  
  if(data.includes("END")) {
    process.exit(0)
  }
})

client.on('close', function() {
    console.log(`client closed`);
})

client.connect(10500, HOST, function() {
  console.log('client connected to server: ' + HOST + ':' + 10500);

  // raw file must be big endian encoded as signed-integer
  var wav_file = "../artifacts/ohayou_gozaimasu.4times.raw"

  const file = fs.createReadStream(wav_file)

  file.pipe(client)
})





