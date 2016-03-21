const mongoose = require('mongoose')
const Schema = mongoose.Schema

const CommentSchema = new Schema({
  comment: String,
  upVotes: { type: Number, default: 0 },
  downVotes: { type: Number, default: 0 }
})

module.exports = mongoose.model('Comment', CommentSchema)
