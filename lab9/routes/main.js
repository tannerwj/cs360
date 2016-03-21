const express = require('express')
const router = express.Router()

const db = require('../config/db')
const Comment = require('../config/comment')

router.get('/comments', function (req, res){
  getComments().then(function (comments){
    res.send(comments)
  })
})

router.post('/save', function (req, res){
  var c = new Comment(req.body)
  c.save().then(function (data){
    res.sendStatus(200)
  })
})

router.post('/delete', function (req, res){
  Comment.findByIdAndRemove({ _id: req.body.id }).then(function (){
    return getComments().then(function (comments){
      res.send(comments)
    })
  })
})

router.post('/upVote', function (req, res){
	Comment.findOneAndUpdate({ _id: req.body.id }, {$inc: { 'upVotes' : 1 }}).then(function (){
  	res.sendStatus(200)
  })
})

router.post('/downVote', function (req, res){
	Comment.findOneAndUpdate({ _id: req.body.id }, {$inc: { 'downVotes' : 1 }}).then(function (){
  	res.sendStatus(200)
  })
})

var getComments = function (){
  return Comment.find({})
}

module.exports = router
