########################
# Flask-restful api example
# city feelings, just a hacky example, but its start
# http://flask-restful.readthedocs.io/en/0.3.5/reqparse.html
# https://blog.miguelgrinberg.com/post/designing-a-restful-api-using-flask-restful

# note: this has no AUTH flow, it should be used on a local network that you trust.
# it is only for testing this proof-of-tech
# Eventully it will parse different external data to arrive at its feeling. 
########################

from flask import Flask, request, Response
from flask_restful import Api, Resource,reqparse, abort
from functools import wraps
import random
from random import choice

app = Flask(__name__)
api = Api(app)

@app.route('/')
def index():
	return "Hello, World!"

@app.route('/about')
def about():
	return "An an about page"

class CityFeels(Resource):
	def getFeels(self):
		""" Construct how your city feels This is just a test for now play w/ this protocol"""
		angryMsgs = [
			"The sky is as red as the blood I will spill from you if you'd don't back off right now.",
			"I could indeed, punch the sun in the face today.",
			"No. Just no. lol.",
			"FUUUUUUUUUUUUUUUUUUUUU....."
		]

		sadMsgs = [
			"I'm just gonna lie here until I die. How about that?",
			"Nope. Nope. Nope. Nope. :( ",
			"All is lost."
		]

		happyMsgs = [
			"WOOOOOO! TODAY IS THE BEST DAY.",
			"TOTES A GOOD DAY Y'ALL!",
			"**runs around in a circle and jumps**"
		]
		aMsg = random.choice(angryMsgs)
		hMsg = random.choice(happyMsgs)
		sMsg = random.choice(sadMsgs)

		randomFeels = [
			{'feeling': "angry",'message':aMsg},
			{'feeling': "happy",'message':hMsg},
			{'feeling': "sad",'message':sMsg},
		]
		return random.choice(randomFeels)

	def get(self):
		boop = self.getFeels()
		#print(boop)
		return boop

api.add_resource(CityFeels, '/cityfeels')

if __name__ == '__main__':
	app.run('0.0.0.0')

