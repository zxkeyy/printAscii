#include "test_helpers.h"
#include "io/image_loader.h"
#include "core/image.h"

#ifndef TEST_IMAGES_DIR
#define TEST_IMAGES_DIR "tests/images"
#endif

#define TEST_IMAGE_PATH(name) TEST_IMAGES_DIR "/" name

TEST_CASE(test_image_load_valid_file) {
    // Load a valid image file
    Image* img = image_load_from_file(TEST_IMAGE_PATH("rgb_100x100.png"));
    ASSERT_NOT_NULL(img);
    ASSERT_EQUAL(img->width, 100);
    ASSERT_EQUAL(img->height, 100);
    ASSERT_EQUAL(img->channels, 3);
    image_free(img);
    return true;
}

TEST_CASE(test_image_load_invalid_file) {
    // Attempt to load an invalid image file
    Image* img = image_load_from_file(TEST_IMAGE_PATH("invalid_image.png"));
    ASSERT_NULL(img);
    return true;
}

TEST_CASE(test_image_load_unsupported_format) {
    // Attempt to load an unsupported image format
    Image* img = image_load_from_file(TEST_IMAGE_PATH("rgb_100x100.webp"));
    ASSERT_NULL(img);
    return true;
}

TEST_CASE(test_image_load_correct_type) {
    // Grayscale image should load as grayscale
    Image* img_gray = image_load_from_file(TEST_IMAGE_PATH("grayscale_100x100.png"));
    ASSERT_NOT_NULL(img_gray);
    ASSERT_EQUAL(img_gray->type, IMAGE_TYPE_GRAY);
    ASSERT_EQUAL(img_gray->channels, 1);
    image_free(img_gray);
    // RGB image should load as RGB
    Image* img_rgb = image_load_from_file(TEST_IMAGE_PATH("rgb_100x100.png"));
    ASSERT_NOT_NULL(img_rgb);
    ASSERT_EQUAL(img_rgb->type, IMAGE_TYPE_RGB);
    ASSERT_EQUAL(img_rgb->channels, 3);
    image_free(img_rgb);
    // RGBA image should load as RGBA
    Image* img_rgba = image_load_from_file(TEST_IMAGE_PATH("rgba_100x100.png"));
    ASSERT_NOT_NULL(img_rgba);
    ASSERT_EQUAL(img_rgba->type, IMAGE_TYPE_RGBA);
    ASSERT_EQUAL(img_rgba->channels, 4);
    image_free(img_rgba);
    return true;
}