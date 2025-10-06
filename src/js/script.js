const home = document.querySelector(".home");
const form = document.querySelector(".formLink");

home.addEventListener("mouseenter", () => {
  home.classList.add("active");
});

document.addEventListener("visibilitychange", () => {
  if (document.hidden) {
    home.classList.remove("active"); // Remove effect when user switches tab
  }
});
form.addEventListener("mouseenter", () => {
  form.classList.add("active");
});

document.addEventListener("visibilitychange", () => {
  if (document.hidden) {
    form.classList.remove("active"); // Remove effect when user switches tab
  }
});
