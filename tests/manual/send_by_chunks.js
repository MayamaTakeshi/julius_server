var net = require('net');
var fs = require('fs');

var HOST = '192.168.225.131';

var client = new net.Socket();

const BUFFER_SIZE = 1600

client.on('end', function() {
    console.log("client end")
})

client.on('error', function (e) {
    console.log(`client error: ${e.code}`);
})

client.on('data', function(data) {
    console.log(`${new Date()} client data: ${data}`)
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
    var file = "../artifacts/ohayou_gozaimasu.4times.raw"

    const fd = fs.openSync(file, "r")

    const buffer = new Buffer(BUFFER_SIZE)

    var tid = setInterval(() => {
        fs.read(fd, buffer, 0, BUFFER_SIZE, null, (err, bytesRead, data) => {
            if(err) {
                console.error(err)
                process.exit(1)
            } else if(bytesRead == 0) {
                console.log("No more data from file.")
                clearInterval(tid)

                // need to close our side of the socket to end read() call at the server.
                client.end()

                /*
                console.log("Sending silence from now on")
                for(i=0 ;i<BUFFER_SIZE; i++) {
                    buffer[i] = 0x00
                }

                tid = setInterval(() => {
                    client.write(buffer)
                }, 20)
                */
            } else {
                console.log(`Fetched ${bytesRead} bytes from audio_file.`)
                client.write(buffer)
            }
        })
    }, 20)
})





