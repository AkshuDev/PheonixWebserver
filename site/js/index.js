document.getElementById('year').textContent = new Date().getFullYear();

const btn = document.getElementById('themeToggle');
btn?.addEventListener('click', () => {
    const root = document.documentElement;
    const isViolet = root.getAttribute('data-theme') == 'violet';
    root.setAttribute('data-theme', isViolet ? '' : 'violet');
});