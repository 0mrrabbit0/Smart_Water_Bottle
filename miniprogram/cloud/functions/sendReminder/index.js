const cloud = require('wx-server-sdk')
cloud.init({ env: cloud.DYNAMIC_CURRENT_ENV })
const db = cloud.database()
const _ = db.command

/**
 * 定时触发器：每30分钟检查是否需要发送饮水提醒
 * config.json 中配置:
 * { "triggers": [{ "name": "drinkReminder", "type": "timer", "config": "0 */30 * * * * *" }] }
 */
exports.main = async (event, context) => {
  const now = new Date()
  const currentHour = now.getHours()
  const currentMinute = now.getMinutes()
  const today = now.toISOString().slice(0, 10)

  // 获取所有用户
  const usersRes = await db.collection('users')
    .where({
      daily_goal: _.gt(0)
    })
    .limit(100)
    .get()

  let sentCount = 0

  for (const user of usersRes.data) {
    // 检查是否在活跃时间内
    const wakeHour = parseInt((user.wake_time || '07:00').split(':')[0])
    const sleepHour = parseInt((user.sleep_time || '23:00').split(':')[0])

    if (currentHour < wakeHour || currentHour >= sleepHour) continue

    // 获取今日汇总
    const summaryRes = await db.collection('daily_summary')
      .where({ user_id: user._id, date: today })
      .get()

    const summary = summaryRes.data[0]
    const totalAmount = summary ? summary.total_amount : 0
    const goal = user.daily_goal || 2000
    const lastDrink = summary ? summary.last_drink : null

    // 已达标则不提醒
    if (totalAmount >= goal) continue

    // 检查上次饮水时间 (超过2小时才提醒)
    if (lastDrink) {
      const lastHour = parseInt(lastDrink.split(':')[0])
      const lastMin = parseInt(lastDrink.split(':')[1])
      const lastMinutes = lastHour * 60 + lastMin
      const currentMinutes = currentHour * 60 + currentMinute
      if (currentMinutes - lastMinutes < 120) continue
    }

    // 发送订阅消息 (需要用户已授权)
    try {
      const remaining = goal - totalAmount
      await cloud.openapi.subscribeMessage.send({
        touser: user._id,
        templateId: '', // 需替换为实际模板ID
        data: {
          thing1: { value: '该喝水啦' },
          number2: { value: remaining },
          time3: { value: now.toLocaleTimeString() }
        }
      })
      sentCount++
    } catch (e) {
      // 用户未授权或模板ID未设置，忽略
    }
  }

  return { sent: sentCount, checked: usersRes.data.length }
}
