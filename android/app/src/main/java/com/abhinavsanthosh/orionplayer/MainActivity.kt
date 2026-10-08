/*
 * Orion Player — Android Edition
 * Copyright (C) 2026 Abhinav Santhosh (GitHub: @abhinavsanthoshpp)
 * All Rights Reserved.
 *
 * Exclusive Intellectual Property of Abhinav Santhosh.
 */

package com.abhinavsanthosh.orionplayer

import android.Manifest
import android.content.ContentUris
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.provider.MediaStore
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.EditText
import android.widget.ImageView
import android.widget.TextView
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.abhinavsanthosh.orionplayer.databinding.ActivityMainBinding

data class VideoItem(
    val id: Long,
    val title: String,
    val duration: Long,
    val uri: Uri
)

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private val videoList = mutableListOf<VideoItem>()
    private lateinit var adapter: VideoAdapter

    private val permissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestPermission()
    ) { granted ->
        if (granted) {
            loadVideos()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setSupportActionBar(binding.toolbar)

        adapter = VideoAdapter(videoList) { item ->
            val intent = Intent(this, PlayerActivity::class.java).apply {
                data = item.uri
                putExtra("title", item.title)
            }
            startActivity(intent)
        }

        binding.recyclerView.layoutManager = LinearLayoutManager(this)
        binding.recyclerView.adapter = adapter

        binding.swipeRefresh.setOnRefreshListener {
            loadVideos()
            binding.swipeRefresh.isRefreshing = false
        }

        binding.toolbar.setOnMenuItemClickListener { menuItem ->
            when (menuItem.itemId) {
                R.id.action_stream -> {
                    showStreamDialog()
                    true
                }
                R.id.action_about -> {
                    showAboutDialog()
                    true
                }
                else -> false
            }
        }

        checkAndRequestPermissions()
    }

    private fun checkAndRequestPermissions() {
        val permission = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            Manifest.permission.READ_MEDIA_VIDEO
        } else {
            Manifest.permission.READ_EXTERNAL_STORAGE
        }

        if (ContextCompat.checkSelfPermission(this, permission) == PackageManager.PERMISSION_GRANTED) {
            loadVideos()
        } else {
            permissionLauncher.launch(permission)
        }
    }

    private fun loadVideos() {
        videoList.clear()
        val projection = arrayOf(
            MediaStore.Video.Media._ID,
            MediaStore.Video.Media.TITLE,
            MediaStore.Video.Media.DURATION
        )

        val sortOrder = "${MediaStore.Video.Media.DATE_ADDED} DESC"

        contentResolver.query(
            MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
            projection,
            null,
            null,
            sortOrder
        )?.use { cursor ->
            val idCol = cursor.getColumnIndexOrThrow(MediaStore.Video.Media._ID)
            val titleCol = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.TITLE)
            val durCol = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DURATION)

            while (cursor.moveToNext()) {
                val id = cursor.getLong(idCol)
                val title = cursor.getString(titleCol) ?: "Unknown Video"
                val duration = cursor.getLong(durCol)
                val uri = ContentUris.withAppendedId(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, id)
                videoList.add(VideoItem(id, title, duration, uri))
            }
        }

        adapter.notifyDataSetChanged()
        binding.emptyView.visibility = if (videoList.isEmpty()) View.VISIBLE else View.GONE
    }

    private fun showStreamDialog() {
        val input = EditText(this).apply {
            hint = "https://example.com/video.m3u8"
            setSingleLine()
            setPadding(48, 32, 48, 32)
        }

        AlertDialog.Builder(this)
            .setTitle(R.string.open_stream)
            .setMessage(R.string.enter_stream_url)
            .setView(input)
            .setPositiveButton("Play") { _, _ ->
                val url = input.text.toString().trim()
                if (url.isNotEmpty()) {
                    val intent = Intent(this, PlayerActivity::class.java).apply {
                        data = Uri.parse(url)
                        putExtra("title", url.substringAfterLast('/'))
                    }
                    startActivity(intent)
                }
            }
            .setNegativeButton("Cancel", null)
            .show()
    }

    private fun showAboutDialog() {
        AlertDialog.Builder(this)
            .setTitle(R.string.about_title)
            .setMessage(R.string.about_desc)
            .setPositiveButton("OK", null)
            .show()
    }

    inner class VideoAdapter(
        private val items: List<VideoItem>,
        private val onItemClick: (VideoItem) -> Unit
    ) : RecyclerView.Adapter<VideoAdapter.ViewHolder>() {

        inner class ViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
            val title: TextView = itemView.findViewById(R.id.videoTitle)
            val duration: TextView = itemView.findViewById(R.id.videoDuration)
            val thumb: ImageView = itemView.findViewById(R.id.videoThumbnail)
        }

        override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
            val view = LayoutInflater.from(parent.context).inflate(R.layout.item_video, parent, false)
            return ViewHolder(view)
        }

        override fun onBindViewHolder(holder: ViewHolder, position: Int) {
            val item = items[position]
            holder.title.text = item.title
            holder.duration.text = formatDuration(item.duration)
            holder.itemView.setOnClickListener { onItemClick(item) }
        }

        override fun getItemCount() = items.size

        private fun formatDuration(ms: Long): String {
            val totalSec = ms / 1000
            val s = totalSec % 60
            val m = (totalSec / 60) % 60
            val h = totalSec / 3600
            return if (h > 0) String.format("%02d:%02d:%02d", h, m, s)
            else String.format("%02d:%02d", m, s)
        }
    }
}
