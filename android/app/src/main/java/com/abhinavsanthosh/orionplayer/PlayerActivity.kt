/*
 * Orion Player — Android Edition
 * Copyright (C) 2026 Abhinav Santhosh (GitHub: @abhinavsanthoshpp)
 * All Rights Reserved.
 *
 * Exclusive Intellectual Property of Abhinav Santhosh.
 */

package com.abhinavsanthosh.orionplayer

import android.app.PictureInPictureParams
import android.content.Context
import android.content.res.Configuration
import android.media.AudioManager
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Rational
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.View
import android.view.WindowManager
import android.widget.Button
import android.widget.ImageButton
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.media3.common.MediaItem
import androidx.media3.common.PlaybackParameters
import androidx.media3.exoplayer.ExoPlayer
import androidx.media3.ui.AspectRatioFrameLayout
import com.abhinavsanthosh.orionplayer.databinding.ActivityPlayerBinding
import kotlin.math.abs

class PlayerActivity : AppCompatActivity() {

    private lateinit var binding: ActivityPlayerBinding
    private var player: ExoPlayer? = null
    private lateinit var audioManager: AudioManager
    private val handler = Handler(Looper.getMainLooper())
    private val osdHideRunnable = Runnable { binding.osdBadge.visibility = View.GONE }

    private var currentSpeed = 1.0f
    private val speeds = floatArrayOf(0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 2.0f)
    private var aspectIndex = 0
    private val aspectModes = intArrayOf(
        AspectRatioFrameLayout.RESIZE_MODE_FIT,
        AspectRatioFrameLayout.RESIZE_MODE_FILL,
        AspectRatioFrameLayout.RESIZE_MODE_ZOOM
    )
    private val aspectNames = arrayOf("Fit", "Fill (Stretch)", "Zoom (Crop)")

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.setFlags(
            WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
            WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS
        )

        binding = ActivityPlayerBinding.inflate(layoutInflater)
        setContentView(binding.root)

        audioManager = getSystemService(Context.AUDIO_SERVICE) as AudioManager

        initPlayer()
        setupCustomControls()
        setupGestures()
    }

    private fun initPlayer() {
        val uri = intent.data ?: return

        player = ExoPlayer.Builder(this).build().apply {
            setMediaItem(MediaItem.fromUri(uri))
            prepare()
            playWhenReady = true
        }

        binding.playerView.player = player
    }

    private fun setupCustomControls() {
        val btnSpeed = binding.playerView.findViewById<Button>(R.id.btnSpeed)
        val btnAspect = binding.playerView.findViewById<ImageButton>(R.id.btnAspect)
        val btnPip = binding.playerView.findViewById<ImageButton>(R.id.btnPip)

        btnSpeed?.setOnClickListener {
            val items = speeds.map { "${it}x" }.toTypedArray()
            AlertDialog.Builder(this)
                .setTitle(R.string.speed)
                .setItems(items) { _, which ->
                    currentSpeed = speeds[which]
                    player?.playbackParameters = PlaybackParameters(currentSpeed)
                    btnSpeed.text = "${currentSpeed}x"
                    showOsd("⚡ Speed: ${currentSpeed}x")
                }
                .show()
        }

        btnAspect?.setOnClickListener {
            aspectIndex = (aspectIndex + 1) % aspectModes.size
            binding.playerView.resizeMode = aspectModes[aspectIndex]
            showOsd("📐 Aspect: ${aspectNames[aspectIndex]}")
        }

        btnPip?.setOnClickListener {
            enterPipMode()
        }
    }

    private fun enterPipMode() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val params = PictureInPictureParams.Builder()
                .setAspectRatio(Rational(16, 9))
                .build()
            enterPictureInPictureMode(params)
        }
    }

    override fun onPictureInPictureModeChanged(isInPictureInPictureMode: Boolean, newConfig: Configuration) {
        super.onPictureInPictureModeChanged(isInPictureInPictureMode, newConfig)
        binding.playerView.useController = !isInPictureInPictureMode
    }

    private fun showOsd(text: String) {
        handler.removeCallbacks(osdHideRunnable)
        binding.osdBadge.text = text
        binding.osdBadge.visibility = View.VISIBLE
        handler.postDelayed(osdHideRunnable, 1200)
    }

    private fun setupGestures() {
        val gestureDetector = GestureDetector(this, object : GestureDetector.SimpleOnGestureListener() {

            override fun onDoubleTap(e: MotionEvent): Boolean {
                val screenWidth = resources.displayMetrics.widthWidth()
                if (e.x < screenWidth * 0.35) {
                    seekRelative(-10000)
                    showOsd("⏪ -10s")
                } else if (e.x > screenWidth * 0.65) {
                    seekRelative(10000)
                    showOsd("⏩ +10s")
                } else {
                    player?.let {
                        if (it.isPlaying) it.pause() else it.play()
                        showOsd(if (it.isPlaying) "▶ Playing" else "⏸ Paused")
                    }
                }
                return true
            }

            override fun onScroll(e1: MotionEvent?, e2: MotionEvent, distanceX: Float, distanceY: Float): Boolean {
                if (e1 == null) return false
                val screenWidth = resources.displayMetrics.widthWidth()

                // Horizontal seek swipe
                if (abs(distanceX) > abs(distanceY) * 2) {
                    val deltaMs = (-distanceX * 200).toLong()
                    seekRelative(deltaMs)
                    val sign = if (deltaMs >= 0) "+" else ""
                    showOsd("⏩ ${sign}${deltaMs / 1000}s")
                    return true
                }

                // Vertical swipe: Left side = Brightness, Right side = Volume
                if (abs(distanceY) > abs(distanceX)) {
                    if (e1.x < screenWidth * 0.5) {
                        adjustBrightness(distanceY)
                    } else {
                        adjustVolume(distanceY)
                    }
                    return true
                }

                return false
            }
        })

        binding.playerView.setOnTouchListener { _, event ->
            gestureDetector.onTouchEvent(event)
            false
        }
    }

    private fun seekRelative(deltaMs: Long) {
        player?.let {
            val newPos = (it.currentPosition + deltaMs).coerceIn(0, it.duration.coerceAtLeast(0))
            it.seekTo(newPos)
        }
    }

    private fun adjustVolume(deltaY: Float) {
        val maxVol = audioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC)
        val curVol = audioManager.getStreamVolume(AudioManager.STREAM_MUSIC)
        val step = if (deltaY > 0) 1 else -1
        val newVol = (curVol + step).coerceIn(0, maxVol)
        audioManager.setStreamVolume(AudioManager.STREAM_MUSIC, newVol, 0)
        val pct = (newVol * 100) / maxVol
        showOsd("🔊 Volume: $pct%")
    }

    private fun adjustBrightness(deltaY: Float) {
        val lp = window.attributes
        var curBrightness = lp.screenBrightness
        if (curBrightness < 0) curBrightness = 0.5f
        val step = if (deltaY > 0) 0.05f else -0.05f
        val newBrightness = (curBrightness + step).coerceIn(0.01f, 1.0f)
        lp.screenBrightness = newBrightness
        window.attributes = lp
        val pct = (newBrightness * 100).toInt()
        showOsd("☀️ Brightness: $pct%")
    }

    private fun android.util.DisplayMetrics.widthWidth(): Int = widthPixels

    override fun onDestroy() {
        super.onDestroy()
        player?.release()
        player = null
    }
}
