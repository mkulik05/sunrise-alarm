let getAlarmsList = async () => {
  let resp = await fetch("/alarms-list/")
  let data = new Uint8Array(await resp.arrayBuffer())
  let res = []
  if (data.length > 0) {
    let i, j

    for (i = 0, j = data.length; i < j; i += 4) {
        res.push(data.slice(i, i + 4));
    }

  }
  return res;
}

let refreshAlarms = async (N = 0) => {
  let alarms = await getAlarmsList()
  for (let i = N; i < alarms.length; i++) {
    let alarm = document.createElement('div')
    alarm.innerHTML = alarms[i]
    alarm.setAttribute("id", `alarm${i}`);
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
        await refreshAlarms(alarmN)
        document.getElementById("alarmAdder").hidden = true
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


let alarmN = 0;
alarmsList = document.getElementById("alarms-list")
document.getElementById("create-alarm").onclick = () => document.getElementById("alarmAdder").hidden = false;
document.getElementById("add-alarm").onclick = addAlarm
document.getElementById("delete-alarms").onclick = deleteAllAlarms;

let selectTime = document.getElementById("select-time")

let days = []

for (let i = 0; i < 7; i++) {
  days.push(document.getElementById("day" + i))
}

refreshAlarms()