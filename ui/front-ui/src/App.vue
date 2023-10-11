<script setup>
const tableData = [
  {
    device_id: "haha",
    name: 'ssh',
    local_ip : "127.0.0.1",
    local_port : 22,
    remote_port: 13145
  },
  {
    device_id: "haha",
    name: 'ssh',
    local_ip : "127.0.0.1",
    local_port : 22,
    remote_port: 13145
  }
]

async function fetchData() {
  try {
    const response = await fetch('http://127.0.0.1:8889/get_devices');
    
    if (!response.ok) {
      throw new Error('网络请求失败');
    }
    
    const data = await response.json();
    console.log(data); // 处理响应数据
  } catch (error) {
    console.error(error);
  }
}

setInterval(() => {
  // console.log("jacklin")
  fetchData()

}, 1000);

</script>

<template>
  <el-table :data="tableData" style="width: 100% height: 100%">
    <el-table-column prop="device_id" label="设备ID" width="180" />
    <el-table-column label="服务信息" width="180" >
      <template #default="scope">
        <el-space direction="vertical">
          <el-text>name : {{ scope.row.name }}</el-text>
          <el-text>local ip : {{ scope.row.local_ip }}</el-text>
          <el-text>local port : {{ scope.row.local_port }}</el-text>
          <el-text>remote port : {{ scope.row.remote_port }}</el-text>
        </el-space>
      </template>
    </el-table-column>
  </el-table>
</template>

<style scoped>
</style>
