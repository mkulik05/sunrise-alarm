let getAlarmsList = async () => {
  let resp = await fetch("/alarms-list/")
  let data = new Uint8Array(await resp.arrayBuffer())
  let res = []
  if (data.length > 0) {
    let i, j

    for (i = 0, j = data.length; i < j; i += 4) {
        res.push([i, ...data.slice(i, i + 4)]);
    }

  }
  return res
  return [
    new Uint8Array([1, 1, 127, 19, 56]),
    new Uint8Array([0, 1, 42, 19, 45]),
    new Uint8Array([2, 1, 2, 20, 25]),
    new Uint8Array([2, 1, 2, 13, 25])
  ];
}

let changeToogleState = (id) => {
  if (!document.querySelector(`.toggle-input.alarm-id-${id}`).checked) {
    document.querySelector(`.alarm-time.alarm-id-${id}`).style.color = "#808294"
    document.querySelector(`.name.alarm-id-${id}`).style.color = "#808294"
  } else {
    document.querySelector(`.alarm-time.alarm-id-${id}`).style.color = "black"
    document.querySelector(`.name.alarm-id-${id}`).style.color = "black"
  }
}

let getAlarmHTML = (id) => {

  let resHTML = document.createElement('div')
  resHTML.setAttribute('class', `alarm-block alarm-id-${id}`)

  let time_name_days = document.createElement('div')
  time_name_days.setAttribute("class", `time-name-days alarm-id-${id}`);

  let time_name = document.createElement('div');
  time_name.setAttribute("class", `time-name alarm-id-${id}`);

  let time = document.createElement('div');
  time.setAttribute("class", `alarm-time alarm-id-${id}`);

  let name = document.createElement('div')
  name.setAttribute("class", `name alarm-id-${id}`)

  time_name.appendChild(time);
  time_name.appendChild(name);

  let days = document.createElement('div');
  days.setAttribute("class", `days alarm-id-${id}`);

  time_name_days.appendChild(time_name)
  time_name_days.appendChild(days)

  let label = document.createElement('label')
  label.setAttribute("class", `toggle-switch alarm-id-${id}`);
  let span = document.createElement('span')
  span.setAttribute("class", `toggle-slider alarm-id-${id}`);
  let toggle = document.createElement('input')
  toggle.addEventListener('click', () => changeToogleState(id));
  toggle.setAttribute("class", `toggle-input alarm-id-${id}`);
  toggle.type = 'checkbox'
  toggle.checked = true;


  label.appendChild(toggle)
  label.appendChild(span)


  resHTML.appendChild(time_name_days);
  resHTML.appendChild(label);

  return resHTML
}

let compareTime = (a, b) => {
  if (a[3] > b[3]) {
    return 1
  }
  if (a[3] < b[3]) {
    return -1
  }
  if (a[4] < b[4]) {
    return -1
  }

  if (a[4] > b[4]) {
    return 1
  }

  return 0;
}

let refreshAlarms = async () => {
  let alarms = await getAlarmsList()
  alarms.sort(compareTime)
  alarmsList.innerHTML = '';
  for (let i = 0; i < alarms.length; i++) {
    let alarm = getAlarmHTML(alarms[i][0])
    let msg = ""
    if (alarms[i][2] == 127) {
      msg = "Daily"
    } else if (alarms[i][2] == 31) {
      msg = "Mon to Fri"
    } else if (alarms[i][2] == 96) {
      msg = "Weeknds"
    } else {
      for (let b = 0; b < 7; b++) {
        if ((2**b & alarms[i][2]) != 0) {
          msg += wdaysNames[b] + '  '
        }
      }
    }

    alarm.querySelector(".days").innerText = msg
    let h = alarms[i][3] > 9 ? alarms[i][3] : "0" + alarms[i][3]
    let m = alarms[i][4] > 9 ? alarms[i][4] : "0" + alarms[i][4]
    alarm.querySelector(".alarm-time").innerText = h + ':' + m

    alarm.querySelector(".toggle-input").checked = true
    alarm.setAttribute("id", `alarm${i}`);
    alarm.setAttribute("class", `alarm-block`);
    alarmsList.appendChild(alarm)
  }  
  alarmN = alarms.length;
}

let addAlarm = async () => {
  let res = 0;
  
  for (let i = 0; i < days.length; i++) {
    let day = days[i];
    if (day.checked) {
      res += 2 ** i
    }
  }
  if (res == 0) {
    alert("Select at lest one day")
  } else {
    let value = selectTime.value 
    let time = [0, 0]
    if (value !== "") {
      time = value.split(":")
      const data = new Uint8Array(3);
      data[0] = res
      data[1] = parseInt(time[0])
      data[2] = parseInt(time[1])

      const response = await fetch("/add-alarm/", {
        method: 'POST',
        headers: {
          'Content-Type': 'application/x-binary'
        },
        body: data 
      });
      res = response.text()
      if (res != "error") {
        await refreshAlarms()
        popup(false, 'none')
      }

    } else {
      alert("Specify time")
    }
  }
}

let deleteAllAlarms = async () => {
await fetch("/remove-alarms/", {
    method: 'POST'
  });
}


let popup = (show, styles) => {
  document.getElementById("alarmAdder").style.display = !show? 'none' : 'flex'
    const elements = document.querySelectorAll('.alarm-block');

  for (const element of elements) {
      element.style.filter = styles;
  }

  document.getElementById("create-alarm").style.filter = styles;
}





let alarmN = 0;
alarmsList = document.getElementById("alarms-list")
document.getElementById("create-alarm").onclick = () => popup(true, 'blur(5px)')

document.getElementById("add-alarm").onclick = addAlarm
document.getElementById("cancel").onclick = () => popup(false, 'none')

document.getElementById("delete-alarms").onclick = deleteAllAlarms;

let selectTime = document.getElementById("select-time")

const wdaysNames = ['Mon', 'Tue', 'Wen', 'Thu', 'Fri', 'Sat', 'Sun']

let days = [];

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



refreshAlarms()