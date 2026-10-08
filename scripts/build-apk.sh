#!/usr/bin/env bash
# Orion Player Android APK Build Script
# Copyright (C) 2026 Abhinav Santhosh (GitHub: @abhinavsanthoshpp)
# All Rights Reserved.

set -e

echo "=========================================================="
echo "  Building Orion Player for Android (APK)"
echo "  Copyright (C) 2026 Abhinav Santhosh. All Rights Reserved."
echo "=========================================================="

cd android

if [ -f "./gradlew" ]; then
    chmod +x ./gradlew
    ./gradlew assembleRelease assembleDebug
else
    gradle assembleRelease assembleDebug
fi

echo "=========================================================="
echo "✅ Build Complete!"
echo "APK output directory: android/app/build/outputs/apk/"
echo "=========================================================="
