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
});
