Component({
  properties: {
    level: {
      type: Number,
      value: 0
    }
  },
  observers: {
    level(val) {
      const fillMap = { 0: 0, 1: 33, 2: 66, 3: 100 }
      const textMap = { 0: '空杯', 1: '低', 2: '中', 3: '满' }
      this.setData({
        fillHeight: fillMap[val] || 0,
        levelText: textMap[val] || '未知'
      })
    }
  },
  data: {
    fillHeight: 0,
    levelText: '空杯'
  }
})
