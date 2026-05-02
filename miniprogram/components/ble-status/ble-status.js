Component({
  properties: {
    connected: { type: Boolean, value: false },
    deviceName: { type: String, value: '' }
  },
  methods: {
    onTap() {
      this.triggerEvent('tap')
    }
  }
})
