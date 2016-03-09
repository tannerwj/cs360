const http = require('http')
const express = require('express')
const bodyParser = require('body-parser')
const path = require('path')

const db = require('./config/db')

var app = express()

app.set('port', process.env.PORT || 3010)
app.use(bodyParser.json())
app.use(bodyParser.urlencoded({ extended: true }))

app.use(express.static('public'))

app.post('/comment', function (req, res){
	var comment = {name: req.body.name, comment: req.body.comment}
	db.saveComment(comment).then(function (){
		res.sendStatus(200)
	}).catch(function (err){
		res.sendStatus(400)
	})
})

app.delete('/comment', function (req, res){
	db.deleteComment(req.body.comment).then(function (){
		res.sendStatus(200)
	}).catch(function (err){
		console.log('err', err)
		res.sendStatus(400)
	})
})

app.get('/comment', function (req, res){
	db.getComments().then(function (comments){
		res.send(comments)
	}).catch(function (err){
		res.sendStatus(400)
	})
})

http.createServer(app).listen(app.get('port'), function(){
	console.log('Server started on port ' + app.get('port'))
})
