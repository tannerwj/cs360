const mongoose = require('mongoose')

mongoose.connect('mongodb://localhost/comments')

var db = mongoose.connection

db.on('error', console.error.bind(console, 'DB connection error:'))

db.once('open', console.error.bind(console, 'DB connected'))

var getDB = function (){
  return new Promise(function (resolve, reject){
    if(db) return resolve(db)
    setTimeout(getDB, 100)
  })
}

module.exports = getDB()
