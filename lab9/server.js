const http = require('http')
const express = require('express')
const bodyParser = require('body-parser')

var app = express()
var port = process.env.PORT || 3000

app.use(bodyParser.json())
app.use(bodyParser.urlencoded({ extended: true }))

app.all('/*', require('./routes/main'))
app.use(express.static('public'))

http.createServer(app).listen(port, function(){
	console.log('HTTP server started on port', port)
})
