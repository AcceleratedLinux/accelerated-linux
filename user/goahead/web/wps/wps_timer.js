var secs
var timerID = null
var timerRunning = false
var timeout = 3
var delay = 1000

var STATimerFlag = 0

function InitializeTimer()
{
    // Set the length of the timer, in seconds
//	timeout = default
    secs = timeout
    StopTheClock()
    StartTheTimer()
}

function StopTheClock()
{
    if(timerRunning)
        clearTimeout(timerID)
    timerRunning = false
}

function StartTheTimer()
{
    if (secs==0)
    {
		StopTheClock()

		if(STATimerFlag == 0){
			updateWPS();
		}else{
			updateWPSStaStatus();
		}

		secs = timeout
		StartTheTimer()
    }else{
        self.status = secs
        secs = secs - 1
        timerRunning = true
        timerID = self.setTimeout("StartTheTimer()", delay)
    }
}
