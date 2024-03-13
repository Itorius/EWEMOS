function openDialog() {
    const dialog = document.querySelector('dialog');
    dialog.showModal();
}

function closeDialog() {
    const dialog = document.querySelector('dialog');
    dialog.close();
}

document.getElementById("minus").addEventListener("click", function (event) {
    document.querySelector('input[type=number]').stepDown();
    event.preventDefault();
});

document.getElementById("plus").addEventListener("click", function (event) {
    document.querySelector('input[type=number]').stepUp();
    event.preventDefault();
});