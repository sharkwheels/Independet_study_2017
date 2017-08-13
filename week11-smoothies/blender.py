#####################################
# Home brew for alexa to decide if they want to make a smoothie or not
###################################

import datetime
import os
import logging
import random
import urllib.request
import json
import time

from random import randint, choice

from flask import Flask, render_template
from flask_ask import Ask, request, session, question, statement

from flask_restful import Resource, Api

from flask_httpauth import HTTPBasicAuth
from werkzeug.security import generate_password_hash, check_password_hash

from threading import Thread
from time import sleep

app = Flask(__name__)
ask = Ask(app, "/")
api = Api(app)
auth = HTTPBasicAuth()

logging.getLogger('flask_ask').setLevel(logging.DEBUG)

## HELPER FUNCTIONS #######################################

def isItAfterNoon():
	# Check to see if its after 12:00 noon. Return bool. 
	minTime = datetime.time(0,0,0)
	now = datetime.datetime.now()
	current = datetime.time(now.hour, now.minute, now.second)
	#print(current, minTime)
	if(current >= minTime):
		return True
	else:
		return False

def getWeather():
	f = urllib.request.urlopen('http://api.wunderground.com/api/XXXXXXX/geolookup/conditions/q/[postalcode].json')
	json_string = f.read().decode('utf-8')
	parsed_json = json.loads(json_string)

	location = parsed_json['location']['city'].lower()
	temp_c = parsed_json['current_observation']['temp_c']
	weather = parsed_json['current_observation']['weather'].lower()
	humidity = parsed_json['current_observation']['relative_humidity'].lower()
	feels_like = parsed_json['current_observation']['feelslike_c']

	weather_list = [location, temp_c, weather, humidity,feels_like]

	f.close()
	return weather_list

def makeBaseMoodNum():
	"""This picks a base mood number between 1 (shit) and 10 (amazing) then augments it based on weather"""
	random.seed()
	baseMood = random.randint(1,8)
	print("starting mood: {}".format(baseMood))

	current_weather = getWeather()
	print(current_weather)

	c_temp = current_weather[1]
	c_weather = current_weather[2]
	c_humidity = current_weather[3]

	### Adjust mood base don temperature
	print("temp: {}".format(c_temp))

	if c_temp < 0:
		baseMood -= 1
	elif c_temp > 0 and c_temp < 24:
		baseMood += 2
	elif c_temp > 24 and c_temp < 30:
		baseMood -= 1 
	elif c_temp > 30:
		baseMood -= 2

	print("aftertemp: {}".format(baseMood))

	## Adjust mood based on humidity
	num_humid = int(c_humidity.replace("%",""))
	print("humidity: {} %".format(num_humid))

	if num_humid < 55:
		baseMood += 2 
	elif num_humid > 55:
		baseMood -= 2 

	print("afterhumid: {}".format(baseMood))

	## adulust mood based on observational weather
	print("weather: {}".format(c_weather))

	downers = ["rain","rainy","storm","snow"]
	uppers = ["sunny","overcast","cloudy"]

	if any(w in c_weather for w in downers):
		baseMood -= 1
	elif any(w in c_weather for w in uppers):
		baseMood += 1
	else:
		pass

	print("after weather: {}".format(baseMood))

	return baseMood

def chooseMood():
	""" Choose the final mood based on the final number """
	final_mood = makeBaseMoodNum()
	print("your mood number is: {}".format(final_mood))

	if final_mood < 1:
		return "very upset"
	elif final_mood == 1 or final_mood == 2:
		return "upset"
	elif final_mood == 3 or final_mood == 4:
		return "low"
	elif final_mood == 5 or final_mood == 6:
		return "neutral"
	elif final_mood == 7 or final_mood == 8:
		return "good"
	elif final_mood == 9 or final_mood == 10:
		return "great"
	elif final_mood > 10: 
		return "amazing"
	else:
		return "neutral"

## API ######################################################

@app.route('/')
def index():
	return "Hello, World! this a basic rest API to toss things to the Feather. It needs some basic Auth."

@app.route('/about')
def about():
	return "An an about page."

class AlexaWants(Resource):
	global resource_var
	resource_var = None

	global state
	state = "off"
	
	def __init__(self):
		self.variable = "default" #instance attribute.

	def get(self):
		if(resource_var):
			return {'command': resource_var,'state':state}
		else:
			return {'command': self.variable, 'state':"off"}


api.add_resource(AlexaWants, '/alexawants')

# Function to be called when the timer expires
def resetFunction():
	global state
	global resource_var
	state = "off"
	resource_var = "default"
	print('ran the reset')
	
# Function with the timer
def resetTimer(seconds):
	sleep(seconds)
	resetFunction()

## INTENTS ############################################################

@ask.launch
def launch():
	return question("<speak><break time='1s' />Say, do you feel like a smoothie?</speak>").reprompt("Sorry, I didn't get that. Do you want me to make smoothies?")

@ask.intent("BlendIntent")
def blend():
	
	afternoon = isItAfterNoon()
	global resource_var
	global state

	resetThread = Thread(target=resetTimer, args=(20,)) ## X seconds
	resetThread.start()

	if(afternoon):
		mood = chooseMood()
		print("final mood: {}".format(mood))
		#return question(burp).reprompt("Do you want a smoothie?")
		if mood == "very upset" or mood == "upset":
			## turn on some neo pixels
			resource_var = "pixels"
			state = "on"
			return statement("I'm too sad to make smoothies, maybe these lights will help?")
		elif mood == "low" or mood == "neutral":
			## play a song
			resource_var = "song"
			state = "on"
			return statement("<speak>I don't feel like a smoothie right now, I think I'd like to meditate for a bit first.<audio src='https://s3.amazonaws.com/soundfxforthings/birds.mp3'/></speak>")
		elif mood == "good" or "great":
			## turn on the blender and make a smothie
			resource_var = "blender"
			state="on"
			return statement("I'm gonna make us smoothies, and its gonna be great!")
		elif mood == "amazing":
			## turn on all the things at once. 
			resource_var = "party"
			state = "on"
			return statement("Wooo! Party!")
		else:
			return statement(mood)
	else:
		return statement("Its too early, come back later.")

@ask.intent('AMAZON.HelpIntent')
def help():
	help_text = render_template('help')
	return question(help_text).reprompt(help_text)

@ask.intent('AMAZON.StopIntent')
def stop():
	bye_text = render_template('bye')
	return statement(bye_text)

@ask.intent('AMAZON.CancelIntent')
def cancel():
	bye_text = render_template('bye')
	return statement(bye_text)

@ask.intent('AMAZON.ResumeIntent')
def resume():
	resume_text = render_template('resume')
	return statement(resume_text)

@ask.intent('AMAZON.PauseIntet')
def pause():
	pause_text = render_template('pause')
	return statement(pause_text)

if __name__ == '__main__':
	if 'ASK_VERIFY_REQUESTS' in os.environ:
		verify = str(os.environ.get('ASK_VERIFY_REQUESTS', '')).lower()
		if verify == 'false':
			app.config['ASK_VERIFY_REQUESTS'] = False
	app.run(host="0.0.0.0",debug=True, use_reloader=False)	### note this has no auth! 
