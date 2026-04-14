Do later:
- [ ] Revisit command sending in general. Each send command should have a corresponding expected change, backoff time, and retry count. We can keep it on the queue and retry it every change we get until we get the expected change.
- [ ] Pre-buffer the outbound message so its ready to go as fast as possible once we receive the right bytes - not sure how timing specific it is