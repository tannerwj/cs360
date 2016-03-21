const http = require('http')
const https = require('https')
const express = require('express')
const passport = require('passport')
const bodyParser = require('body-parser')
const LocalStrategy = require('passport-local').Strategy
const cookieParser = require('cookie-parser')
const session = require('express-session')
const fs = require('fs')

var app = express()

app.use(bodyParser.json())
app.use(bodyParser.urlencoded({ extended: true }))
app.use(cookieParser())

passport.use(new LocalStrategy(function (user, pass, done) {
  if(user === 'lab8' && pass === 'lab8'){
    done(null, {user: user})
  }else{
    done(null, false)
  }
}))
passport.serializeUser(function (user, done) {
	done(null, user.user)
})
passport.deserializeUser (function(user, done) {
 done(null, {user: user})
})

app.use(session({ secret: 'so secret', resave: false, saveUninitialized: true }))
app.use(passport.initialize())
app.use(passport.session())

app.all('/*', require('./routes/all'))
app.use(express.static('public'))

http.createServer(app).listen(3008, function(){
	console.log('HTTP server started on port 3008')
})

var options = {
  host: '54.213.137.33',
  key: fs.readFileSync('ssl/server.key'),
  cert: fs.readFileSync('ssl/server.crt')
}

https.createServer(options, app).listen(3009, function(){
	console.log('HTTPS server started on port 3009')
})
