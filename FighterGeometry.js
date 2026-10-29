.pragma library

// Model offsets are authored in sprite pixels. Scale the bottom anchor as well
// as the image, including animation padding, jump arcs and visualScale offsets.
function spriteY(modelY, viewportHeight, frameHeight, fitScale) {
    return viewportHeight + (modelY - viewportHeight + frameHeight) * fitScale - frameHeight;
}
