const alarmSize = 26
const wdaysNames = ['Mon', 'Tue', 'Wen', 'Thu', 'Fri', 'Sat', 'Sun']

// Assistive functions declaraation

const compareTime = (a, b) => {
  if (a[3] > b[3]) return 1
  if (a[3] < b[3]) return -1
  if (a[4] < b[4]) return -1
  if (a[4] > b[4]) return 1

  return 0;
}

const isASCII = (str) => {
  return str.charCodeAt(0) >= 0 && str.charCodeAt(0) <= 127;
}

const isInRange = (n, range) => {
  if (!Number.isInteger(n)) return 0
  if (n < range[0] || n > range[1]) return 0
  return 1
}


// Controls popup window (to add/edit alarm)
const popup = (show, styles, edit = false, id=0) => {
  alarmAdder.style.display = !show? 'none' : 'flex'
  const elements = document.querySelectorAll('.alarm-block');
  for (const element of elements) {
      element.style.filter = styles;
  }

  createAlarm.style.filter = styles
  removeBTN.style.display = edit ? 'block' : 'none'
  backBTN.style.display = edit ? 'none' : 'block'
  clickBlocker.style.display = show ? 'block' : 'none'
  if (!edit) {
    fillFormEmpty()
    alarmID = alarmsN + 1
  } else {
    removeBTN.onclick = () => removeAlarm(id)
    alarmID = id
    fillForm(id)
  }
}

// Clears creation form for adding new alarm
const fillFormEmpty = () => {
  for (let i = 0; i < days.length; i++) {
    days[i].checked = false;
  }
  selectTime.value = ''
  alarmName.value = ''
  riseTimeSelect.value = ''
  workAfterTimeSelect.value = ''
}

// Resrotes form content based on alarm settings
const fillForm = (id) => {
  let alarm = alarms[id - 1]
  if (alarm[0] !== id) {
    for (let i = 0; i < alarms.length; i++) {
      if (alarms[i] == id) {
        alarm = alarms[i]
        break
      }
    }
  }
  for (let i = 0; i < days.length; i++) {
    if ((2**i & alarm[2]) != 0) {
      days[i].checked = true;
    } else {
      days[i].checked = false;
    }
  }
  let h = alarm[3] > 9 ? alarm[3] : "0" + alarm[3]
  let m = alarm[4] > 9 ? alarm[4] : "0" + alarm[4]
  selectTime.value = h + ":" + m
  alarmName.value = String.fromCharCode(...alarm.slice(7));
  riseTimeSelect.value = alarm[5]
  workAfterTimeSelect.value = alarm[6]
}

// Post request with included password
// returns server response
const request = async (url, data=new Uint8Array()) => {
  let webPwd = localStorage.getItem("password")
  let pwd = new Uint8Array(webPwd.length)

  for (let i = 0; i < webPwd.length; i++) {
    pwd[i] = webPwd[i].charCodeAt()
  }  

  const resData = new Uint8Array(data.length + 8);
  resData.set(data, 0);
  resData.set(pwd.slice(0, 8), data.length);
    
  const resp = await fetch(url, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/x-binary'
    },
    body: resData
  })

  if (resp.status == 403) {
    passwordPromt.style.display = 'flex'
    return ""
  } else {
    return resp
  }
}

// Return html for alarm block, that will be added to alarms-list
// No innerHTML - no XSS
const getAlarmHTML = (id) => {

  let resHTML = document.createElement('div')
  resHTML.setAttribute('class', `alarm-block alarm-id-${id} space-between`)
  resHTML.setAttribute("id", `alarm${id}`);

  let delays_time_name_days = document.createElement('div')
  delays_time_name_days.setAttribute("class", `delays-time-name-days alarm-id-${id}`);
  delays_time_name_days.onclick = () => popup(true, 'blur(5px)', edit=true, id=id)

  let delays_time_name = document.createElement('div');
  delays_time_name.setAttribute("class", `delays-time-name wrapper alarm-id-${id}`);

  let delays_time = document.createElement('div');
  delays_time.setAttribute("class", `delays-time wrapper alarm-id-${id}`);

  let delays = document.createElement('div');
  delays.setAttribute("class", `delays wrapper alarm-id-${id} space-between`);

  let delayRise = document.createElement('div');
  delayRise.setAttribute("class", `delay-rise alarm-id-${id}`);

  let delayAfter = document.createElement('div');
  delayAfter.setAttribute("class", `delay-after alarm-id-${id}`);

  delays.appendChild(delayRise)
  delays.appendChild(delayAfter)

  let time = document.createElement('div');
  time.setAttribute("class", `time alarm-id-${id}`);

  delays_time.appendChild(delays)
  delays_time.appendChild(time)

  let name = document.createElement('div')
  name.setAttribute("class", `name alarm-id-${id}`)

  delays_time_name.appendChild(delays_time);
  delays_time_name.appendChild(name);

  let days = document.createElement('div');
  days.setAttribute("class", `days alarm-id-${id}`);

  delays_time_name_days.appendChild(delays_time_name)
  delays_time_name_days.appendChild(days)

  let label = document.createElement('label')
  label.setAttribute("class", `toggle-switch alarm-id-${id}`);

  let span = document.createElement('span')
  span.setAttribute("class", `toggle-slider alarm-id-${id}`);

  let toggle = document.createElement('input')
  toggle.addEventListener('click', () => changeToogleStateServer(id));
  toggle.setAttribute("class", `toggle-input alarm-id-${id}`);
  toggle.type = 'checkbox'
  toggle.checked = true;

  label.appendChild(toggle)
  label.appendChild(span)

  resHTML.appendChild(delays_time_name_days);
  resHTML.appendChild(label);

  return resHTML
}


// Functions which communicate with server
const getAlarmsList = async () => {
  let resp = await request("/alarms-list/")
  let res = []
  if (resp !== '') {
    let data = new Uint8Array(await resp.arrayBuffer())
    if (data.length > 0) {
      let i, j, n
      n = 1;
      for (i = 0, j = data.length; i < j; i += alarmSize) {
          res.push([n, ...data.slice(i, i + alarmSize)]);
          n++
      }
  
    }
  }
  return res
}

// Change alarm colors
const updateAlarmState = (isChecked, id) => {
  document.querySelector(`.time.alarm-id-${id}`).style.color = isChecked ? 'black' : "#808294"
  document.querySelector(`.name.alarm-id-${id}`).style.color = isChecked ? '#717171' : "#808294"
  document.querySelector(`.delays.alarm-id-${id}`).style.color = isChecked ? '#717171' : "#808294"
  document.querySelector(`.toggle-input.alarm-id-${id}`).checked = isChecked
}

// Called on changing state of toggle, sending request to server to enable/disable alarm
// on successful server answer change color af alarm using updateAlarmState func
const changeToogleStateServer = async (id) => {
  const isChecked = document.querySelector(`.toggle-input.alarm-id-${id}`).checked
  const data = new Uint8Array(2);
  data[0] = id
  data[1] = isChecked ? 1 : 0
  const response = await request("/turn-alarm-on-off/", data)
  const res = await response.text()
  if (res == "OK") {
    updateAlarmState(isChecked, id)
  }
}

// Checks entered password, on correct answer hides password prompt and save password in local storage
const checkPassword = async (password) => {
  let data = new Uint8Array(password.length)
  for (let i = 0; i < password.length; i++) {
    data[i] = password[i].charCodeAt()
  }
  const resp = await fetch('/check-password/', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/x-binary'
    },
    body: data
  })
  const statusCode = resp.status
  const ok = (statusCode == 200)
  if (ok) {
    localStorage.setItem('password', password);
    refreshAlarms()
  }
  passwordPromt.style.display = ok ? 'none' : 'flex'
  incorrectPwdLabel.style.display = ok ? 'none' : 'block'
}

const refreshAlarms = async () => {
  alarms = await getAlarmsList() // alarms is global var
  let sorted = alarms.map((x) => x); // full copy of alarms
  sorted.sort(compareTime)
  alarmsList.innerHTML = '';
  for (let i = 0; i < sorted.length; i++) {
    const alarmId = sorted[i][0]
    let alarm = getAlarmHTML(alarmId)
    alarmsList.appendChild(alarm)

    let isEnabled = sorted[i][1]
    
    let msg = ""
    if (sorted[i][2] == 127) {
      msg = "Daily"
    } else if (sorted[i][2] == 31) {
      msg = "Mon to Fri"
    } else if (sorted[i][2] == 96) {
      msg = "Weeknds"
    } else {
      for (let b = 0; b < 7; b++) {
        if ((2**b & sorted[i][2]) != 0) {
          msg += wdaysNames[b] + '  '
        }
      }
    }

    const h = sorted[i][3] > 9 ? sorted[i][3] : "0" + sorted[i][3]
    const m = sorted[i][4] > 9 ? sorted[i][4] : "0" + sorted[i][4]

    // From index 7 cause array element is alarm' ID
    let resName = []
    let char = ""
    for (let j = 7; j < sorted[i].length; j++) {
      char = sorted[i][j]
      if (char == 0) break
      resName.push(char)
    }

    alarm.querySelector(".days").innerText = msg
    alarm.querySelector(".time").innerText = h + ':' + m
    alarm.querySelector(".delay-rise").innerText = sorted[i][5]
    alarm.querySelector(".delay-after").innerText =  sorted[i][6]
    alarm.querySelector(`.name`).innerText = String.fromCharCode(...resName);

    updateAlarmState(isEnabled ? 1 : 0, alarmId)
  }  
  alarmsN = sorted.length;
}


const saveAlarm = async () => {
  let res = 0;
  
  for (let i = 0; i < days.length; i++) {
    let day = days[i];
    if (day.checked) {
      res += 2 ** i
    }
  }
  if (res == 0) {
    alert("Select at least one day")
    return
  }
  const timeValue = selectTime.value
  if (timeValue == "") {
    alert("Specify time")
    return
  }
  const time = timeValue.split(":")
  const data = new Uint8Array(27);
  data[0] = alarmID
  data[1] = 1;
  data[2] = res
  data[3] = parseInt(time[0])
  data[4] = parseInt(time[1])

  const riseTime = parseInt(riseTimeSelect.value)
  const workAfterTime = parseInt(workAfterTimeSelect.value)
  if (isInRange(riseTime, [1, 240]) && isInRange(workAfterTime, [1, 240])) {
    data[5] = riseTime;
    data[6] = workAfterTime;
  } else {
    alert("<Time rise> and <Stay after rise> values should be between 1 and 240")
    return
  }
  const name = alarmName.value
  if (name != "") {
    for (let i = 0; i < name.length; i++) {
      if (!isASCII(name[i])) {
        alert("Name may include only ASCII symbols")
        return
      }
      data[7 + i] = name[i].charCodeAt()
    }
  }
  const response = await request("/save-alarm/", data)
  await refreshAlarms()
  popup(false, 'none')
}

const removeAlarm = async (id) => {
  if (id <= alarmsN) {
    const data = new Uint8Array(1)
    data[0] = id
    const response = await request("/remove-alarm/", data)
    await refreshAlarms()
    popup(false, 'none')
  }
}


// alarms - list of alarms
// alarmsN - their amount

// alarmID - id of alarm, user currently interacting with
// days - array of days checkboxes from alarm add/edit form

let alarms = []
let alarmsN = 0;
let alarmID = 0;
let days = [];

// Getting all important fields/chechboxes/so on once
const alarmsList = document.getElementById("alarms-list")
const riseTimeSelect = document.getElementById("time-rise")
const workAfterTimeSelect = document.getElementById("time-work-after")
const alarmName = document.getElementById("alarm_name")
const passwordPromt = document.getElementById("password-promt")
const passwordInput = document.getElementById("passwordInput")
const incorrectPwdLabel = document.getElementById("pwdError")
const backBTN = document.getElementById("cancel")
const removeBTN = document.getElementById("remove")
const clickBlocker = document.getElementById("clickBlocker")
const selectTime = document.getElementById("select-time")
const alarmAdder = document.getElementById("alarmAdder")
const createAlarm = document.getElementById("create-alarm")

// Assigning events handlers 
document.getElementById("create-alarm").onclick = () => popup(true, 'blur(5px)')
document.getElementById("save-alarm").onclick = saveAlarm
document.getElementById("savePassword").onclick = () => checkPassword(passwordInput.value)
document.getElementById("delete-alarms").onclick = () => request("/remove-alarms/");
clickBlocker.addEventListener('click', () => popup(false, 'none'));
backBTN.onclick = () => popup(false, 'none')

// Generating week days selection for alarm editing/creation form
for (let i = 0; i < wdaysNames.length; i++) {
  let inp = document.createElement('input');
  inp.setAttribute("type", `checkbox`);
  inp.setAttribute("id", `day${i}`);
  days.push(inp)
  let label = document.createElement('label');
  label.setAttribute("for", `day${i}`)
  label.innerText = wdaysNames[i]
  document.getElementById("alarm-day-select").appendChild(inp)
  document.getElementById("alarm-day-select").appendChild(label)
}

// If there is saved password in local storage, checking it automatically
// In other case -- showing it
if (localStorage.getItem("password")) {
  checkPassword(localStorage.getItem("password"))
} else {
  passwordPromt.style.display = 'flex'
}