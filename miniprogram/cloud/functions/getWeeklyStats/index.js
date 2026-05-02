const cloud = require('wx-server-sdk')
cloud.init({ env: cloud.DYNAMIC_CURRENT_ENV })
const db = cloud.database()
const _ = db.command

exports.main = async (event, context) => {
  const { OPENID } = cloud.getWXContext()

  // 最近7天日期
  const dates = []
  for (let i = 6; i >= 0; i--) {
    const d = new Date()
    d.setDate(d.getDate() - i)
    dates.push(d.toISOString().slice(0, 10))
  }

  const res = await db.collection('daily_summary')
    .where({
      user_id: OPENID,
      date: _.in(dates)
    })
    .orderBy('date', 'asc')
    .get()

  // 构建完整 7 天数据 (无记录的天补 0)
  const dataMap = {}
  res.data.forEach(d => { dataMap[d.date] = d })

  return dates.map(date => {
    const d = dataMap[date]
    return {
      date,
      amount: d ? d.total_amount : 0,
      goal: d ? d.goal : 2000,
      achieved: d ? d.goal_achieved : false
    }
  })
}
