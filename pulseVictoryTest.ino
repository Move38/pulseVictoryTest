enum pulseStates {INERT, CHECK, FAIL, RESOLVE};
byte pulseState = INERT;

bool isCool = false;
Timer checkTimer;
#define CHECK_TIME 1000
bool checkFailed = true;

bool isCelebrating = false;

void setup() {
  // put your setup code here, to run once:
  randomize();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (pulseState == INERT) {//listen for clicks & CHECKs

    //scenario 1 - THE CLICK
    if (buttonSingleClicked()) {
      isCool = !isCool;

      //send out the pulse
      if (isCool == true) {
        pulseState = CHECK;
        checkTimer.set(CHECK_TIME);
        checkFailed = false;
      } else if (isCool == false) {
        pulseState = FAIL;
        checkFailed = true;
        isCelebrating = false;
      }
    }

    //scenario 2 - THE PULSE
    //listen for pulses, potential spread or contradict them
    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
        if (getLastValueReceivedOnFace(f) == CHECK) {//this neighbor is sending a CHECK pulse
          if (isCool == true) {
            pulseState = CHECK;
            checkTimer.set(CHECK_TIME);
            checkFailed = false;
          } else if (isCool == false) {
            pulseState = FAIL;
            checkFailed = true;
          }
        } else if (getLastValueReceivedOnFace(f) == FAIL) {
          pulseState = FAIL;
          checkFailed = true;
        }
      }
    }
  } else if (pulseState == CHECK) {

    bool inertNeighbors = false;
    bool failNeighbors = false;
    FOREACH_FACE(f) {//figure out if I can RESOLVE or should go to FAIL
      if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
        if (getLastValueReceivedOnFace(f) == INERT) {
          inertNeighbors = true;
        } else if (getLastValueReceivedOnFace(f) == FAIL) {
          failNeighbors = true;
        }
      }
    }

    //actually use the results of the loop
    if (failNeighbors == true) {
      pulseState = FAIL;
      checkFailed = true;
    } else if (inertNeighbors == true) {
      pulseState = CHECK;
    } else {
      pulseState = RESOLVE;
    }

  } else if (pulseState == FAIL) {
    //neighbors in INERT or CHECK are bad
    bool badNeighbors = false;

    FOREACH_FACE(f) {//figure out if I can RESOLVE or should go to FAIL
      if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
        if (getLastValueReceivedOnFace(f) == INERT || getLastValueReceivedOnFace(f) == CHECK) {
          badNeighbors = true;
        }
      }
    }

    //use results
    if (badNeighbors == false) {
      pulseState = RESOLVE;
    }

  } else if (pulseState == RESOLVE) {
    //neighbors in FAIL or CHECK are bad
    bool badNeighbors = false;

    FOREACH_FACE(f) {//figure out if I can RESOLVE or should go to FAIL
      if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
        if (getLastValueReceivedOnFace(f) == CHECK || getLastValueReceivedOnFace(f) == FAIL) {
          badNeighbors = true;
        }

        if (getLastValueReceivedOnFace(f) == FAIL && checkFailed == false)  {//I'm hearing fail, but I myself never failed
          checkFailed = true;
          pulseState = FAIL;
        }
      }
    }

    //use results
    if (badNeighbors == false && pulseState == RESOLVE) {//if I am still in resolve and I have no bad neighbors
      pulseState = INERT;
    }
  }

  //CHECK THE TIMER
  if (checkTimer.isExpired()) {
    if (checkFailed) {
      isCelebrating = false;
    } else {
      isCelebrating = true;
    }
  }

  //communication!
  setValueSentOnAllFaces(pulseState);

  //display!
  //baseline isCool display
  if (isCool) {
    setColor(dim(WHITE, 150));
  } else {
    setColor(OFF);
  }

  //pulse state display
  switch (pulseState) {
    case INERT:
      setColorOnFace(WHITE, 0);
      break;
    case CHECK:
      setColorOnFace(YELLOW, 0);
      break;
    case FAIL:
      setColorOnFace(RED, 0);
      break;
    case RESOLVE:
      setColorOnFace(CYAN, 0);
      break;
  }

  //celebration
  if (isCelebrating) {
    setColorOnFace(ORANGE, random(5));
  }
}
