// script.js

document.addEventListener('DOMContentLoaded', () => {
    const fInput = document.getElementById('fileInput');
    const pBar = document.getElementById('progressBar');
    const pText = document.getElementById('progressText');
    const fName = document.getElementById('fileName');
    const modal = document.getElementById('myModal');
    const cModal = document.getElementById('closeModal');
    const uImage = document.getElementById('uploadedImageModal');
    const pContainer = document.getElementById('previewContainer');
    const cBtn = document.getElementById('clearButton');
    fInput.addEventListener('change', (event) => {
        const file = event.target.files[0];
        if (file && file.type.startsWith('image/')) {
            const reader = new FileReader();
            reader.onloadstart = () => {
                pBar.style.width = '0%';
                pText.style.display = 'block';
                pText.innerText = '0%';
                pContainer.style.display = 'none';
                cBtn.style.display = 'none';
            };
            reader.onprogress = (event) => {
                if (event.lengthComputable) {
                    const progress = 
                        (event.loaded / event.total) * 100;
                    pBar.style.width = `${progress}%`;
                    pText.innerText = `${Math.round(progress)}%`;
                }
            };
            reader.onload = () => {
                const uploadTime = 4000;
                const interval = 50;
                const steps = uploadTime / interval;
                let currentStep = 0;
                const updateProgress = () => {
                    const progress = (currentStep / steps) * 100;
                    pBar.style.width = `${progress}%`;
                    pText.innerText = `${Math.round(progress)}%`;
                    currentStep++;

                    if (currentStep <= steps) {
                        setTimeout(updateProgress, interval);
                    } else {
                        pBar.style.width = '100%';
                        pText.innerText = '100%';
                        fName.innerText = `File Name: ${file.name}`;
                        pContainer.innerHTML = 
                            `<img src="${reader.result}" 
                                  alt="Preview" id="previewImage">`;
                        pContainer.style.display = 'block';
                        cBtn.style.display = 'block';
                    }
                };
                setTimeout(updateProgress, interval);
            };
            reader.readAsDataURL(file);
        } else {
            alert('Please select a valid image file.');
            fInput.value = '';
        }
    });
    pContainer.addEventListener('click', () => {
        modal.style.display = 'block';
        uImage.src = document.getElementById('previewImage').src;
    });
    cModal.addEventListener('click', () => {
        modal.style.display = 'none';
    });
    cBtn.addEventListener('click', () => {
        fInput.value = '';
        pBar.style.width = '0%';
        pText.style.display = 'none';
        fName.innerText = '';
        pContainer.style.display = 'none';
        cBtn.style.display = 'none';
    });
    window.addEventListener('click', (event) => {
        if (event.target === modal) {
            modal.style.display = 'none';
        }
    });
});
