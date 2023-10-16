<template>
  <el-table :data="tableData" :border="true" style="width: 100%">
    <el-table-column type="expand">
      <template #default="props">
        <div m="4">
          <el-table :data="props.row.info" :border="true">
            <el-table-column label="name" prop="name" />
            <el-table-column label="local ip" prop="local_ip" />
            <el-table-column label="local port" prop="local_port" />
            <el-table-column label="remote port" prop="remote_port" />
          </el-table>
        </div>
      </template>
    </el-table-column>
    <el-table-column width="500px" label="Login Date" prop="date" />
    <el-table-column width="500px" label="ID" prop="ID" />
  </el-table>
</template>

<script lang="ts" setup>
import { ref } from 'vue'
let tableData = ref([]);
function arraysAreEqual(arr1, arr2) {
  return JSON.stringify(arr1) === JSON.stringify(arr2);
}

async function fetchData() {
  try {
    const response = await fetch('http://172.16.33.155:8889/get_devices'); // 替换为您要请求的URL
    if (!response.ok) {
      throw new Error('网络请求失败');
    }
    const data = await response.json();
    if( !arraysAreEqual(data["devices"], tableData.value) ){
      tableData.value = data["devices"]
    }
  } catch (error) {
    console.error('发生错误:', error);
  }
}

setInterval(()=>{
  fetchData()
}, 1000)

</script>
