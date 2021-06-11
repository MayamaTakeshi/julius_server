const net = require('net')
const fs = require('fs')
const m = require('moment')

const SpeechRecogStream = require('speech-recog-stream')

var argv = require('yargs/yargs')(process.argv.slice(2))
    .usage(`
Usage: $0 [-i server_ip_address] [-p server_port] [-l] [-b buffer_size] raw_audio_file
Ex:    $0 ../artifacts/ohayou_gozaimasu.4times.raw
`)
    .default('i', 'localhost')
    .default('p', '10500')
    .default('b', '320')
    .option('l', {
      demand: false,
      describe: 'indicates if we should loop sending data from the the audio file'
    })
    .boolean('l')
    .demandCommand(1)
    .argv

var host = argv.i
var port = parseInt(argv.p)
var buffer_size = parseInt(argv.b)
var file = argv._[0]
var loop = argv.l

var tid = null

const log = msg => {
    console.log(`${m().format("YYYY-MM-DD HH:mm:ss.SSS")} : ${msg}`)
}

const opts = {
    server_ip: '0.0.0.0',
    server_port: 10500,
}

const srs = new SpeechRecogStream(opts)
srs.on('data', data => {
    log(JSON.stringify(data))
})

srs.on('ready', () => {
    log('ready')
    start_audio_transmission()
})

srs.on('error', err => {
    log(`error: ${err}`)
    if(tid) {
        clearInterval(tid)
        tid = null
    }
})

srs.on('close', err => {
    log('close')
    if(tid) {
        clearInterval(tid)
        tid = null
    }
})

const start_audio_transmission = () => {
    log(`starting audio transmission from ${file}`)
    const fd = fs.openSync(file, "r")

    const buffer = Buffer.alloc(buffer_size)

    tid = setInterval(() => {
        fs.read(fd, buffer, 0, buffer_size, null, (err, bytesRead, data) => {
            if(err) {
                log(err)
                process.exit(1)
            } else if(bytesRead == 0) {
                log("No more data from file.")
                clearInterval(tid)
                tid = null

                if(loop) {
                    start_audio_transmission()
                }
            } else {
                //log(`Fetched ${bytesRead} bytes from audio_file.`)

                srs.write(data)
            }
        })
    }, 20)
}

