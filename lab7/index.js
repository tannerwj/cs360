const express = require('express')
const fs = require('fs')
const request = require('request')

var app = express()
var cities

app.disable('x-powered-by')
app.use(express.static(__dirname + '/public'))

app.get('/cities', function (req, res){
	var pre = req.query.q || 'nothing'
	var arr = cities.reduce( function (arr, city){
		if(city.substring(0, pre.length) === pre){
			arr.push({'city': city})
		}
		return arr
	}, [])
	res.send(arr)
})

app.get('/joke', function (req, res){
	request('http://api.icndb.com/jokes/random', function (err, response, body){
		var data = JSON.parse(body)
		res.send(data.value.joke)
	})
})

fs.readFile('./cities.dat.txt', 'utf8', function (err, data) {
	cities = data.split('\n')
})

app.listen(3000)
