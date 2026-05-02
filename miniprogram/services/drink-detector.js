const { WATER_LEVEL_CAPACITY } = require('../utils/constants')

/**
 * 自动饮水检测器
 * 通过水位变化推断饮水行为
 */
class DrinkDetector {
  constructor() {
    this.lastLevel = -1
    this.stableLevel = -1
    this.stableTimer = null
    this.stableDelay = 5000 // 水位稳定等待时间 5s
    this._callback = null
    this.detecting = false
  }

  /**
   * 注册饮水检测回调
   * callback(amount, levelBefore, levelAfter)
   */
  onDrinkDetected(callback) {
    this._callback = callback
  }

  /**
   * 输入新的水位数据
   */
  feed(waterLevel) {
    if (waterLevel == null || waterLevel < 0 || waterLevel > 3) return

    // 初始化
    if (this.lastLevel < 0) {
      this.lastLevel = waterLevel
      this.stableLevel = waterLevel
      return
    }

    // 水位下降 → 可能在饮水
    if (waterLevel < this.lastLevel && !this.detecting) {
      this.detecting = true
      this.stableLevel = this.lastLevel // 记住饮水前水位
    }

    // 水位上升 (加水) → 重置
    if (waterLevel > this.lastLevel) {
      this.detecting = false
      if (this.stableTimer) {
        clearTimeout(this.stableTimer)
        this.stableTimer = null
      }
      this.lastLevel = waterLevel
      return
    }

    this.lastLevel = waterLevel

    // 水位稳定计时
    if (this.detecting) {
      if (this.stableTimer) clearTimeout(this.stableTimer)

      this.stableTimer = setTimeout(() => {
        this._onStable(waterLevel)
      }, this.stableDelay)
    }
  }

  /**
   * 水位稳定后，计算饮水量
   */
  _onStable(currentLevel) {
    this.detecting = false
    this.stableTimer = null

    if (currentLevel >= this.stableLevel) return

    // 计算饮水量
    let amount = 0
    for (let level = this.stableLevel; level > currentLevel; level--) {
      amount += WATER_LEVEL_CAPACITY[level] || 100
    }

    if (amount > 0 && this._callback) {
      this._callback(amount, this.stableLevel, currentLevel)
    }

    this.stableLevel = currentLevel
  }

  /**
   * 重置检测器
   */
  reset() {
    this.lastLevel = -1
    this.stableLevel = -1
    this.detecting = false
    if (this.stableTimer) {
      clearTimeout(this.stableTimer)
      this.stableTimer = null
    }
  }
}

module.exports = new DrinkDetector()
