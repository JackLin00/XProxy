import { defineConfig } from 'vite'
import viteCompression from 'vite-plugin-compression'
import vue from '@vitejs/plugin-vue'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [vue(), viteCompression()],
})
