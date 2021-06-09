var net = require('net');
var fs = require('fs');

var HOST = '192.168.225.131';

var client = new net.Socket();

const BUFFER_SIZE = 320

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
                setTimeout(() => {
                    console.log("Terminating")
                    process.exit(0)
                }, 1000)

            } else {
                //console.log(`Fetched ${bytesRead} bytes from audio_file.`)

                for(var i=0 ;i<data.length/2 ; i++) {
                    var temp = data[i*2]
                    data[i*2] = data[i*2+1]
                    data[i*2+1] = temp
                }

                client.write(data)
            }
        })
    }, 20)
})


