# Imports
import requests
import math
from bs4 import BeautifulSoup
from datetime import date, datetime
from datetime import timedelta
import serial
from time import sleep

# Determine if we need yesterday's games or today's
URL = "https://www.cbssports.com/nba/scoreboard/"

# Create necessary variables related to time
now = datetime.now()
today = date.today()
yesterday = today - timedelta(days=1)
today8pm = now.replace(hour = 20, minute = 0, second = 0, microsecond = 0)

# If it is before 8 pm, then refer to yesterday's games
# Did this to make sure that games have had a chance to start for current day
if(now < today8pm):
    URL += yesterday.strftime("%Y%m%d")

# Create BeautifulSoup objects
page = requests.get(URL)
soup = BeautifulSoup(page.content, "html.parser")
total = soup.find("div", class_="score-cards")
teams = total.findAll("a", class_="team")

# Dictionary that holds key/value pairs
# Key: Team, Value: Score
teamScores = {}

# Get the teams that are playing on a given day and associate their scores
teamsPlaying = []

for i in range(len(teams)):
    teamsPlaying.append(teams[i].text)

for i in range(len(teams)):
    if i % 2 == 0:
        teamScores[teamsPlaying[(i + 1) * -1 - 1]] = total.findAll(class_="in-progress-table")[math.floor(((i + 1) * -1) / 2)].findAll("tbody")[0].findAll("tr")[i % 2].findAll("td")[-1].text
    else:
        teamScores[teamsPlaying[(i + 1) * -1 + 1]] = total.findAll(class_="in-progress-table")[math.floor(((i + 1) * -1) / 2)].findAll("tbody")[0].findAll("tr")[i % 2].findAll("td")[-1].text
print(teamScores)

print(list(teamScores.keys()))
print(list(teamScores.values()))

# sleep(3)

def main():
    # Initialize Serial Port Communications
    serialPort = serial.Serial(
        port='COM8',
        baudrate=9600,
        bytesize=8,
        timeout=2,
        stopbits=serial.STOPBITS_ONE
    )

    serialString = ""
    sendBuffer = ""
    i = 0

    # Sending Game Names
    while(i < 6):
        serialString = serialPort.readline()
        # Sending Game Names
        sendBuffer = str.encode("N="+list(teamScores.keys())[i]+"\r\n") # Home Name
        serialPort.write(sendBuffer)
        i = i+1

        #serialPort.write(b"homeScore=169\r\n")
        print(serialString)     # Used for debugging
        sleep(3)

    i = 0
    while(i < 6):
        serialString = serialPort.readline()
        # Sending Game Scores
        sendBuffer = str.encode("S="+ list(teamScores.values())[i]+"\r\n")
        serialPort.write(sendBuffer)
        i = i+1

        print(serialString)     # Used for debugging
        sleep(3)
    print("Data Sent")
    

main()
