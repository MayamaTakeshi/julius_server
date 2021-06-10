const net = require('net')
const fs = require('fs')
const m = require('moment')

var argv = require('yargs/yargs')(process.argv.slice(2))
    .usage(`
Usage: $0 [-i server_ip_address] [-p server_port] [-loop] [-b buffer_size] raw_audio_file
Ex:    $0 ../artifacts/ohayou_gozaimasu.4times.raw
`)
    .default('i', 'localhost')
    .default('p', '10500')
    .default('b', '320')
    .demandCommand(1)
    .argv

var host = argv.i
var port = parseInt(argv.p)
var buffer_size = parseInt(argv.b)
var file = argv._[0]

var client = new net.Socket()
var audio_port_client = client

var send_length_prefix = false

const log = msg => {
    console.log(`${m().format("YYYY-MM-DD HH:mm:ss.SSS")} : ${msg}`)
}

client.on('end', function() {
   log("client end")
})

client.on('error', function (e) {
    log(`client error: ${e.code}`)
})

var acc = ""

client.on('data', function(data) {
    //log(`${new Date()} client data: ${data}`)
    var data = data.toString('utf8')

    acc += data

    var pos = acc.indexOf("\n")
    while(pos >= 0) {
        var msg = acc.substring(0, pos)
        acc = acc.substring(pos+1)

        log(`msg=${msg}`)

        if(msg.startsWith('+AUDIO_PORT:')) {
            var audio_port = parseInt(msg.split(":")[1])
            log(`audio_port=${audio_port}`)

            audio_port_client = new net.Socket()

            audio_port_client.on('end', function() {
                log("audio_port_client end")
            })

            audio_port_client.on('error', function (e) {
                log(`audio_port_client error: ${e.code}`)
            })

            audio_port_client.on('data', function(data) {
                log(`${new Date()} audio_port_client data: ${data}`)
            })

            audio_port_client.on('close', function() {
                log(`audio_port_client closed`)
            })

            audio_port_client.connect(audio_port, host, function() {
                log(`audio_port_client connected to server: ${host}:${audio_port}`)
            })

            send_length_prefix = true
        } else if(msg == '+READY') {
            const fd = fs.openSync(file, "r")

            const buffer = Buffer.alloc(buffer_size)

            var tid = setInterval(() => {
                fs.read(fd, buffer, 0, buffer_size, null, (err, bytesRead, data) => {
                    if(err) {
                        log(err)
                        process.exit(1)
                    } else if(bytesRead == 0) {
                        log("No more data from file.")
                        clearInterval(tid)
                    } else {
                        //log(`Fetched ${bytesRead} bytes from audio_file.`)
                        if(send_length_prefix) {
                            var lenB = Buffer.alloc(4)
                            lenB[0] = data.length & 0xff
                            lenB[1] = (data.length >> 8) & 0xff
                            lenB[2] = (data.length >> 16) & 0xff
                            lenB[3] = (data.length >> 24) & 0xff
                            audio_port_client.write(lenB)
                        }

                        for(var i=0 ;i<data.length/2 ; i++) {
                            var temp = data[i*2]
                            data[i*2] = data[i*2+1]
                            data[i*2+1] = temp
                        }

                        audio_port_client.write(data)
                    }
                })
            }, 20)
        }

        pos = acc.indexOf("\n")
    }
})

client.on('close', function() {
    log(`client closed`)
})

client.connect(port, host, function() {
    log(`client connected to server: ${host}:${port}`)

})


