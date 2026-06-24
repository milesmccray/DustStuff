function openPreview(imageSrc) {
	const modal = document.getElementById('preview-modal');
	const modalImg = document.getElementById('preview-modal-img');
	modalImg.src = imageSrc;
	modal.style.display = 'flex';
}

function closePreview() {
	document.getElementById('preview-modal').style.display = 'none';
}

document.getElementById('preview-modal').addEventListener('click', function (event) {
	if (event.target === this) {
		closePreview();
	}
});