const express = require('express')
const router = express.Router()
const passport = require('passport')
const fs = require('fs')

const db = require('../config/db')
var cities

var auth = function (req, res, next){
  req.isAuthenticated() ? next() : res.sendStatus(401)
}

router.post('/login', function (req, res, next) {
	passport.authenticate('local', function(err, user, info) {
		if(err) { return next(err) }
		if (!user) { return res.sendStatus(401) }

		req.logIn(user, function(err) {
			if (err) { return next(err) }
			return res.sendStatus(200)
		})
	})(req, res, next)
})

router.get('/logout', function (req, res){
  req.logout()
  res.sendStatus(200)
})

router.get('/status', function (req, res){
  res.send('logged ' + (req.isAuthenticated() ? 'in' : 'out'))
})

router.post('/comment', auth, function (req, res){
	var comment = {name: req.body.name, comment: req.body.comment}
	db.saveComment(comment).then(function (){
		res.sendStatus(200)
	}).catch(function (err){
		res.sendStatus(400)
	})
})

router.delete('/comment', auth, function (req, res){
	db.deleteComment(req.body.comment).then(function (){
		res.sendStatus(200)
	}).catch(function (err){
		console.log('err', err)
		res.sendStatus(400)
	})
})

router.get('/comment', function (req, res){
	db.getComments().then(function (comments){
		res.send(comments)
	}).catch(function (err){
		res.sendStatus(400)
	})
})

router.get('/cities', function (req, res){
	var pre = req.query.q || 'nothing'
	var arr = cities.reduce( function (arr, city){
		if(city.substring(0, pre.length) === pre){
			arr.push({'city': city})
		}
		return arr
	}, [])
	res.send(arr)
})

router.get('/joke', function (req, res){
	request('http://api.icndb.com/jokes/random', function (err, response, body){
		var data = JSON.parse(body)
		res.send(data.value.joke)
	})
})

fs.readFile('./cities.dat.txt', 'utf8', function (err, data) {
	cities = data.split('\n')
})

module.exports = router
