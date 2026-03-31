const cloud = require('wx-server-sdk')
cloud.init({ env: cloud.DYNAMIC_CURRENT_ENV })
const db = cloud.database()
const _ = db.command

exports.main = async (event, context) => {
  const { OPENID } = cloud.getWXContext()

  // 确定月份
  let month = event.month
  if (!month) {
    const now = new Date()
    month = `${now.getFullYear()}-${String(now.getMonth() + 1).padStart(2, '0')}`
  }

  // 查询该月所有汇总
  const res = await db.collection('daily_summary')
    .where({
      user_id: OPENID,
      date: _.and(_.gte(`${month}-01`), _.lte(`${month}-31`))
    })
    .orderBy('date', 'asc')
    .limit(31)
    .get()

  const days = res.data.map(d => ({
    date: d.date,
    amount: d.total_amount || 0,
    achieved: d.goal_achieved || false,
    goal: d.goal || 2000
  }))

  const totalAmount = days.reduce((sum, d) => sum + d.amount, 0)
  const activeDays = days.filter(d => d.amount > 0).length
  const achievedDays = days.filter(d => d.achieved).length

  return {
    month,
    days,
    total_amount: totalAmount,
    active_days: activeDays,
    achieved_days: achievedDays,
    goal_rate: activeDays > 0 ? Math.round(achievedDays / activeDays * 100) : 0
  }
}
