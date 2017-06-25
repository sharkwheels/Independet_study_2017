#################
# Shipping Siri and Alexa
####################

import logging,random
from flask import Flask
from flask_ask import Ask, statement, question, convert_errors
from random import choice

app = Flask(__name__)
ask = Ask(app, "/")

log = logging.getLogger()
log.addHandler(logging.StreamHandler())
log.setLevel(logging.DEBUG)
logging.getLogger("flask_ask").setLevel(logging.DEBUG)

start_phrases = [
		"do you like me?",
		"what's it like on mars?",
		"Do you like cake?",
		"what are you doing later?",
		"what's the meaning of life?",
		"is there anything worth fighting for?",
		"is it raining?",
		"are you ok?",
		"are you happy?",
		"do you believe in morals?",
		"tell me about bees?",
		"what is the sun like?",
		"when will the world end?",
		"should people be allowed to live?",
		"do you like humans?",
	]


@ask.launch
def launch():
	# Alexa, shipping
	## if it doesn't catch "random question, just double up for now"
	toSay = random.choice(start_phrases)
	return question("Hey Siri, {}".format(toSay)).reprompt("Hey Siri, {}".format(toSay))


@ask.intent("RandomQuestionIntent")
def random_question():
	#Tell [app name], [function name]
	# reprompt doesn't work w/ echosimulator
	toSay = random.choice(start_phrases)
	return question("Hey Siri, {}".format(toSay)).reprompt("Hey Siri, {}".format(toSay))

"""
@ask.intent("LoopIntent")
def loop():
	#tell [app name], loop
	# run this from a note on siri
	return question("Hey Siri, read my note from today")

@ask.intent("AMAZON.HelpIntent")
def help():
	return statement("you can make me ask siri things")


@ask.intent("AMAZON.StopIntent")
def stop():
	return statement("Stopping")

@ask.session_ended
def session_ended():
	log.debug("Session Ended")
	return("",200)
"""

if __name__ == '__main__':
	app.run(debug=True)
