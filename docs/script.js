document.addEventListener('DOMContentLoaded', () => {
    // 1. One-click Copy for Terminal Command
    const copyBtn = document.getElementById('copyCmdBtn');
    if (copyBtn) {
        copyBtn.addEventListener('click', () => {
            const command = "curl -sSL https://raw.githubusercontent.com/abhinavsanthoshpp/orionplayer/main/install.sh | bash";
            navigator.clipboard.writeText(command).then(() => {
                const originalText = copyBtn.innerText;
                copyBtn.innerText = "✅ Copied!";
                copyBtn.style.color = "#00e5ff";
                setTimeout(() => {
                    copyBtn.innerText = originalText;
                    copyBtn.style.color = "";
                }, 2000);
            }).catch(err => {
                console.error("Failed to copy command: ", err);
            });
        });
    }

    // 2. Distro Download Tabs Switcher
    const tabs = document.querySelectorAll('.distro-tab');
    const contents = document.querySelectorAll('.tab-content');

    tabs.forEach(tab => {
        tab.addEventListener('click', () => {
            const targetId = `tab-${tab.getAttribute('data-tab')}`;

            tabs.forEach(t => t.classList.remove('active'));
            contents.forEach(c => c.classList.add('hidden'));

            tab.classList.add('active');
            const targetContent = document.getElementById(targetId);
            if (targetContent) {
                targetContent.classList.remove('hidden');
            }
        });
    });

    // 3. Smooth Anchor Scrolling
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function (e) {
            const target = document.querySelector(this.getAttribute('href'));
            if (target) {
                e.preventDefault();
                target.scrollIntoView({
                    behavior: 'smooth',
                    block: 'start'
                });
            }
        });
    });

    // 4. Donation Amount Selector on Home Page
    const homeAmountBtns = document.querySelectorAll('#donate .amount-btn');
    const homeCustomInput = document.getElementById('custom-amount-home');
    if (homeAmountBtns.length > 0) {
        homeAmountBtns.forEach(btn => {
            btn.addEventListener('click', () => {
                homeAmountBtns.forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                if (homeCustomInput) homeCustomInput.value = '';
            });
        });
    }

    if (homeCustomInput) {
        homeCustomInput.addEventListener('input', () => {
            if (homeCustomInput.value) {
                homeAmountBtns.forEach(b => b.classList.remove('active'));
            } else if (homeAmountBtns.length > 1) {
                homeAmountBtns[1].classList.add('active');
            }
        });
    }
});

// Global functions for inline onclick handlers
function copyHomeUpi(btn) {
    const upiId = document.getElementById('home-upi-id')?.textContent.trim() || 'abhinava6525@naviaxis';
    navigator.clipboard.writeText(upiId).then(() => {
        const orig = btn.innerText;
        btn.innerText = "✔ Copied!";
        btn.style.background = "#10b981";
        btn.style.borderColor = "#10b981";
        setTimeout(() => {
            btn.innerText = orig;
            btn.style.background = "";
            btn.style.borderColor = "";
        }, 2000);
    }).catch(() => {
        prompt("Copy UPI ID:", upiId);
    });
}

function toggleHomeQr() {
    const qrDisplay = document.getElementById('home-upi-qr');
    const toggleBtn = document.getElementById('home-toggle-qr-btn');
    if (qrDisplay) {
        const isShown = qrDisplay.classList.toggle('show');
        if (toggleBtn) toggleBtn.innerText = isShown ? "✕ Hide QR" : "📱 Scan QR";
    }
}
