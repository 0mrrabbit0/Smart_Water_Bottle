const cloud = require('wx-server-sdk')
cloud.init({ env: cloud.DYNAMIC_CURRENT_ENV })
const db = cloud.database()
const _ = db.command

exports.main = async (event, context) => {
  const { OPENID } = cloud.getWXContext()
  const { amount, source, temperature, tds, water_level_before, water_level_after } = event

  // 参数校验
  if (!amount || amount <= 0 || amount > 5000) {
    return { success: false, message: '无效的饮水量' }
  }

  const now = new Date()
  const dateKey = now.toISOString().slice(0, 10)

  // 1. 写入饮水记录
  await db.collection('drink_records').add({
    data: {
      user_id: OPENID,
      amount: Math.round(amount),
      source: source || 'manual',
      temperature: temperature || null,
      tds: tds || null,
      water_level_before: water_level_before || null,
      water_level_after: water_level_after || null,
      recorded_at: now,
      date_key: dateKey
    }
  })

  // 2. 更新每日汇总
  const summaryRes = await db.collection('daily_summary')
    .where({ user_id: OPENID, date: dateKey })
    .get()

  // 获取用户目标
  const userRes = await db.collection('users').where({ _id: OPENID }).get()
  const goal = (userRes.data[0] && userRes.data[0].daily_goal) || 2000

  if (summaryRes.data.length > 0) {
    // 更新现有汇总
    const summary = summaryRes.data[0]
    const newTotal = (summary.total_amount || 0) + Math.round(amount)
    const newCount = (summary.drink_count || 0) + 1
    const newAvgTemp = temperature
      ? ((summary.avg_temperature || 0) * summary.drink_count + temperature) / newCount
      : summary.avg_temperature
    const newAvgTds = tds
      ? ((summary.avg_tds || 0) * summary.drink_count + tds) / newCount
      : summary.avg_tds

    const timeStr = now.toTimeString().slice(0, 5)

    await db.collection('daily_summary').doc(summary._id).update({
      data: {
        total_amount: newTotal,
        drink_count: newCount,
        goal_achieved: newTotal >= goal,
        avg_temperature: newAvgTemp ? Math.round(newAvgTemp * 10) / 10 : null,
        avg_tds: newAvgTds ? Math.round(newAvgTds) : null,
        last_drink: timeStr
      }
    })

    return {
      success: true,
      daily_total: newTotal,
      goal_progress: Math.min(1, newTotal / goal)
    }
  } else {
    // 创建新汇总
    const timeStr = now.toTimeString().slice(0, 5)

    // 计算连续达标天数
    let streakDays = 0
    const yesterday = new Date(now)
    yesterday.setDate(yesterday.getDate() - 1)
    const yesterdayKey = yesterday.toISOString().slice(0, 10)
    const yesterdaySummary = await db.collection('daily_summary')
      .where({ user_id: OPENID, date: yesterdayKey })
      .get()
    if (yesterdaySummary.data.length > 0 && yesterdaySummary.data[0].goal_achieved) {
      streakDays = (yesterdaySummary.data[0].streak_days || 0) + 1
    }

    const newTotal = Math.round(amount)
    await db.collection('daily_summary').add({
      data: {
        user_id: OPENID,
        date: dateKey,
        total_amount: newTotal,
        drink_count: 1,
        goal: goal,
        goal_achieved: newTotal >= goal,
        avg_temperature: temperature ? Math.round(temperature * 10) / 10 : null,
        avg_tds: tds ? Math.round(tds) : null,
        first_drink: timeStr,
        last_drink: timeStr,
        streak_days: streakDays
      }
    })

    return {
      success: true,
      daily_total: newTotal,
      goal_progress: Math.min(1, newTotal / goal)
    }
  }
}
