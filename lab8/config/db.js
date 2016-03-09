const MongoClient = require('mongodb').MongoClient
const ObjectId = require('mongodb').ObjectID

var url = 'mongodb://localhost:27017/lab8'
var db

var getDB = function(){
	return new Promise(function (resolve, reject){
		if(db) return resolve(db)
		MongoClient.connect(url, function (err, database) {
			if(err) return reject(err)
			db = database
			return resolve(database)
		})
	})
}

exports.saveComment = function (comment){
	return getDB().then(function (db){
		return db.collection('comments').insertOne(comment)
	})
}

exports.getComments = function (){
	return getDB().then(function (db){
		return db.collection('comments').find({}).toArray()
	})
}

exports.deleteComment = function (id){
	return getDB().then(function (db){
		return db.collection('comments').remove({'_id': ObjectId(id)})
	})
}
