#pragma once

double GRAIN_AMP_SLOPES[10][512] =
{
    // Classic
	/*
	 ................... NI ............................................................................
	..................,MMMMM............................................................................
	 ................ZMMMMMMM...........................................................................
	................MMMMMMMMMMN.........................................................................
	 ..............MMMMMMMMMMMMM........................................................................
	.............MMMMMMMMMMMMMMMMO .....................................................................
	 ...........MMMMMMMMMMMMMMMMMMM ....................................................................
	...........MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM? .....................
	 ........MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM ...................
	 ......DNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM .................
	 .... MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMN, .............
	....ZMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM ...........
	 ..MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM.........
	 =MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMN......
	MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM ...
	MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
	*/
	{ 7, 9, 12, 15, 18, 21, 24, 26, 28, 31, 34, 36, 38, 40, 43, 45, 47, 49, 52, 54, 56, 58, 60, 62, 63, 65, 67, 69, 71, 74, 76, 78, 81, 83, 85, 87, 89, 91, 93, 96, 98, 99, 101, 103, 105, 106, 108, 110, 111, 113, 115, 117, 119, 121, 124, 126, 128, 131, 133, 135, 138, 140, 142, 145, 147, 149, 152, 154, 157, 159, 161, 163, 165, 168, 170, 172, 175, 177, 179, 181, 184, 186, 187, 190, 192, 194, 196, 199, 201, 204, 206, 208, 210, 212, 214, 216, 218, 219, 221, 223, 225, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244, 244, 244, 243, 242, 240, 238, 236, 233, 231, 229, 227, 225, 223, 221, 219, 216, 214, 212, 209, 207, 205, 202, 200, 198, 196, 193, 191, 189, 186, 184, 182, 179, 177, 175, 173, 171, 169, 167, 165, 163, 161, 159, 157, 154, 152, 150, 148, 145, 143, 141, 140, 138, 137, 135, 134, 133, 133, 132, 132, 131, 131, 131, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 130, 130, 129, 128, 127, 126, 125, 123, 122, 121, 119, 118, 117, 115, 114, 113, 111, 110, 109, 107, 106, 105, 103, 102, 101, 99, 98, 97, 95, 94, 93, 91, 90, 89, 87, 86, 85, 84, 83, 82, 80, 79, 78, 77, 76, 75, 75, 73, 72, 72, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 55, 54, 53, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 29, 28, 27, 26, 24, 23, 22, 21, 19, 18, 17, 16, 14, 13, 12, 10, 9, 8, 7, 6, 6, 5, 4, 4, 3, 2, 3, 0 },

    // flat top
    { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 6, 7, 8, 10, 11, 13, 15, 16, 18, 20, 23, 25, 28, 30, 33, 36, 39, 42, 45, 48, 51, 55, 58, 62, 65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 106, 110, 114, 119, 124, 128, 133, 138, 142, 147, 152, 157, 161, 166, 170, 175, 179, 183, 187, 191, 195, 198, 202, 205, 208, 210, 213, 215, 217, 219, 221, 222, 224, 225, 226, 227, 228, 229, 230, 231, 231, 232, 232, 233, 233, 233, 233, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 235, 235, 235, 235, 235, 235, 235, 235, 236, 236, 236, 236, 236, 236, 236, 236, 236, 236, 236, 236, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 234, 234, 234, 234, 234, 234, 234, 234, 233, 233, 233, 233, 233, 233, 233, 233, 233, 233, 233, 233, 233, 233, 233, 233, 233, 233, 233, 233, 233, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 236, 236, 236, 236, 236, 236, 236, 236, 236, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 237, 236, 236, 236, 236, 236, 236, 236, 236, 236, 236, 236, 236, 236, 236, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 234, 233, 233, 233, 232, 232, 232, 231, 231, 230, 230, 230, 229, 229, 229, 228, 228, 228, 227, 227, 227, 226, 226, 226, 225, 225, 224, 224, 223, 223, 222, 221, 221, 220, 219, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 207, 206, 205, 203, 202, 200, 198, 197, 195, 193, 191, 190, 188, 186, 185, 183, 181, 180, 178, 176, 175, 173, 171, 169, 168, 165, 163, 161, 158, 156, 153, 149, 146, 143, 139, 135, 131, 127, 123, 119, 115, 111, 107, 103, 100, 96, 92, 88, 85, 81, 78, 74, 70, 66, 62, 58, 54, 50, 46, 42, 38, 34, 30, 26, 22, 19, 16, 14, 11, 9, 7, 6, 5, 4, 3, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },


    // Dip in middle
{ 0, 0, 4, 5, 8, 10, 12, 15, 17, 20, 23, 25, 28, 31, 34, 38, 41, 44, 47, 51, 54, 58, 61, 65, 68, 72, 76, 79, 83, 87, 90, 94, 98, 101, 105, 109, 113, 116, 120, 124, 128, 132, 135, 139, 143, 147, 151, 155, 158, 162, 166, 169, 173, 176, 180, 183, 186, 189, 192, 195, 198, 201, 203, 206, 208, 210, 212, 214, 216, 218, 219, 220, 222, 223, 224, 224, 225, 226, 226, 227, 227, 227, 228, 228, 228, 228, 228, 228, 229, 229, 229, 229, 229, 229, 229, 229, 229, 229, 229, 229, 228, 228, 228, 228, 228, 228, 227, 227, 226, 226, 225, 225, 224, 223, 222, 221, 220, 218, 217, 215, 213, 211, 209, 207, 204, 202, 199, 196, 193, 190, 187, 183, 180, 176, 172, 169, 165, 160, 156, 152, 148, 144, 139, 135, 131, 127, 122, 118, 114, 110, 106, 102, 99, 95, 92, 89, 86, 83, 80, 77, 75, 73, 71, 69, 67, 65, 63, 62, 61, 60, 59, 58, 57, 57, 56, 56, 55, 55, 55, 55, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 55, 56, 56, 56, 56, 57, 57, 58, 58, 59, 59, 60, 60, 61, 62, 63, 64, 65, 66, 67, 68, 70, 71, 72, 74, 76, 77, 79, 81, 82, 84, 86, 88, 90, 92, 95, 97, 99, 101, 104, 106, 109, 112, 115, 117, 120, 124, 127, 130, 134, 137, 141, 145, 148, 152, 156, 160, 164, 168, 172, 175, 178, 182, 184, 187, 189, 192, 194, 195, 197, 199, 200, 202, 203, 204, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 216, 217, 218, 218, 219, 220, 220, 220, 221, 221, 222, 222, 223, 223, 223, 224, 224, 224, 225, 225, 225, 226, 226, 226, 227, 227, 227, 228, 228, 228, 228, 228, 229, 229, 229, 229, 229, 230, 230, 230, 230, 230, 230, 231, 231, 231, 231, 231, 231, 231, 231, 232, 232, 232, 232, 232, 232, 233, 233, 233, 233, 233, 233, 233, 233, 233, 232, 232, 232, 232, 232, 231, 231, 231, 230, 230, 230, 229, 229, 229, 228, 228, 228, 227, 227, 226, 226, 226, 225, 225, 224, 224, 223, 223, 222, 221, 221, 220, 219, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208, 206, 205, 204, 202, 201, 199, 198, 196, 194, 193, 191, 189, 188, 186, 184, 183, 181, 179, 177, 176, 174, 172, 170, 167, 165, 163, 161, 158, 155, 153, 150, 147, 143, 140, 137, 133, 130, 126, 123, 119, 115, 111, 108, 104, 100, 96, 92, 89, 85, 81, 77, 73, 69, 66, 62, 58, 54, 50, 47, 43, 39, 36, 33, 29, 26, 23, 21, 18, 16, 14, 12, 10, 8, 7, 6, 5, 4, 3, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

// Medium

{
0, 0, 1, 1, 2, 3, 5, 6, 9, 11, 15, 18, 22, 26, 30, 35, 39, 44, 48, 53, 57, 62, 67, 71, 76, 80, 84, 89, 93, 97, 101, 105, 109, 113, 117, 120, 124, 128, 131, 135, 138, 142, 145, 149, 152, 156, 159, 163, 167, 171, 174, 178, 182, 187, 191, 195, 199, 203, 207, 211, 215, 218, 221, 222, 223, 223, 221, 219, 214, 209, 203, 195, 187, 179, 170, 162, 153, 145, 137, 129, 122, 115, 108, 102, 96, 90, 85, 81, 76, 72, 69, 65, 62, 59, 56, 54, 51, 49, 47, 45, 43, 42, 40, 39, 37, 36, 35, 34, 33, 32, 32, 31, 30, 29, 28, 28, 27, 26, 25, 25, 24, 23, 23, 22, 22, 21, 21, 21, 20, 20, 20, 20, 20, 19, 19, 19, 19, 18, 18, 18, 18, 17, 17, 17, 17, 16, 16, 16, 16, 15, 15, 15, 15, 14, 14, 14, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 6, 6, 6, 5, 5, 4, 4, 4, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 16, 17, 19, 21, 23, 25, 27, 29, 32, 34, 36, 39, 41, 44, 47, 49, 52, 55, 58, 61, 64, 66, 69, 72, 75, 78, 80, 83, 86, 88, 91, 94, 97, 100, 102, 105, 108, 111, 113, 116, 119, 121, 123, 126, 129, 132, 135, 138, 141, 145, 148, 151, 154, 157, 160, 162, 165, 167, 169, 171, 173, 174, 176, 177, 178, 179, 180, 181, 182, 182, 183, 183, 183, 183, 183, 183, 183, 183, 183, 182, 182, 182, 181, 181, 180, 180, 179, 178, 176, 175, 174, 173, 171, 170, 169, 167, 165, 164, 161, 159, 156, 154, 151, 148, 145, 141, 138, 134, 130, 127, 123, 118, 114, 110, 106, 102, 98, 94, 90, 86, 83, 79, 76, 72, 69, 66, 63, 60, 57, 54, 51, 48, 45, 42, 39, 37, 34, 31, 28, 25, 22, 19, 17, 14, 12, 10, 8, 6, 5, 4, 3, 3, 2, 2, 2, 2, 3
},


{
0, 0, 0, 0, 1, 1, 2, 3, 5, 6, 8, 11, 13, 16, 19, 22, 25, 29, 33, 36, 40, 45, 50, 54, 59, 64, 68, 72, 76, 80, 84, 88, 92, 96, 100, 105, 110, 115, 121, 127, 133, 139, 146, 153, 159, 165, 171, 176, 181, 185, 189, 193, 196, 200, 203, 207, 210, 214, 217, 220, 223, 224, 224, 223, 220, 216, 212, 206, 200, 194, 188, 182, 176, 170, 164, 158, 152, 146, 141, 135, 129, 124, 118, 113, 108, 104, 99, 95, 90, 86, 81, 75, 71, 66, 60, 55, 50, 45, 39, 35, 31, 29, 27, 28, 31, 34, 38, 44, 49, 55, 60, 65, 70, 75, 80, 85, 91, 96, 102, 107, 114, 120, 126, 134, 140, 146, 152, 159, 164, 169, 175, 182, 187, 192, 198, 203, 208, 213, 216, 219, 220, 220, 217, 214, 209, 204, 198, 193, 187, 182, 176, 170, 164, 157, 151, 144, 137, 129, 122, 114, 105, 97, 90, 82, 74, 68, 61, 55, 50, 45, 40, 35, 31, 27, 24, 23, 23, 24, 27, 31, 35, 40, 44, 49, 54, 58, 63, 67, 71, 75, 79, 83, 87, 91, 95, 99, 103, 107, 111, 115, 119, 123, 127, 131, 135, 140, 145, 149, 153, 158, 163, 167, 171, 176, 180, 185, 189, 194, 198, 202, 207, 211, 216, 219, 221, 221, 221, 217, 211, 206, 199, 192, 186, 180, 174, 168, 162, 156, 150, 144, 138, 132, 126, 121, 116, 111, 106, 101, 96, 92, 87, 83, 79, 75, 71, 67, 64, 60, 57, 53, 50, 47, 43, 40, 36, 33, 31, 30, 29, 30, 32, 35, 38, 43, 47, 50, 55, 58, 62, 65, 69, 72, 76, 79, 83, 86, 90, 93, 96, 100, 103, 106, 110, 113, 116, 119, 123, 127, 130, 134, 138, 142, 146, 150, 154, 159, 164, 169, 174, 178, 183, 187, 192, 196, 200, 204, 208, 212, 215, 216, 217, 217, 213, 209, 205, 200, 194, 190, 185, 180, 176, 171, 167, 162, 158, 154, 149, 145, 142, 138, 133, 130, 126, 122, 118, 114, 110, 107, 103, 99, 95, 92, 88, 84, 81, 77, 74, 71, 68, 65, 62, 59, 56, 53, 50, 47, 44, 42, 39, 37, 35, 32, 31, 30, 29, 30, 31, 33, 36, 38, 41, 44, 46, 49, 51, 54, 56, 59, 62, 65, 67, 71, 73, 76, 80, 83, 86, 89, 92, 95, 99, 102, 105, 109, 112, 116, 120, 124, 128, 132, 136, 140, 143, 148, 151, 155, 158, 162, 165, 169, 172, 176, 180, 183, 187, 191, 195, 199, 203, 205, 206, 206, 205, 199, 193, 186, 178, 170, 163, 156, 150, 144, 138, 132, 127, 121, 116, 111, 106, 102, 97, 93, 88, 83, 79, 74, 69, 65, 60, 55, 51, 47, 42, 37, 33, 30, 25, 21, 18, 14, 11, 8, 5, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
},


{
0, 0, 0, 1, 1, 3, 6, 9, 21, 32, 38, 51, 63, 70, 78, 87, 97, 107, 120, 135, 150, 163, 180, 195, 207, 173, 137, 145, 105, 65, 54, 43, 50, 40, 32, 40, 47, 55, 62, 67, 73, 79, 84, 89, 94, 99, 104, 110, 115, 119, 92, 65, 73, 51, 30, 44, 69, 90, 103, 123, 139, 148, 154, 161, 169, 177, 183, 189, 195, 201, 205, 208, 212, 215, 219, 224, 216, 160, 117, 114, 112, 120, 125, 182, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 176, 121, 130, 83, 37, 58, 90, 132, 163, 183, 216, 236, 236, 236, 236, 237, 179, 120, 126, 68, 5, 14, 12, 12, 19, 13, 17, 27, 17, 18, 62, 57, 62, 94, 92, 103, 114, 164, 169, 134, 186, 180, 128, 135, 129, 180, 136, 136, 238, 187, 187, 189, 131, 181, 189, 131, 181, 238, 191, 191, 238, 191, 191, 238, 238, 181, 136, 193, 181, 136, 193, 238, 238, 238, 238, 238, 195, 138, 181, 142, 85, 122, 125, 127, 66, 71, 126, 76, 33, 81, 90, 98, 135, 141, 196, 238, 238, 238, 238, 183, 128, 132, 132, 128, 132, 187, 238, 184, 184, 196, 141, 141, 153, 196, 196, 153, 196, 239, 151, 151, 239, 194, 194, 196, 196, 239, 196, 196, 239, 239, 239, 239, 239, 187, 143, 195, 187, 93, 145, 239, 143, 143, 181, 90, 97, 130, 136, 143, 188, 239, 193, 193, 239, 193, 193, 193, 143, 189, 193, 143, 142, 192, 239, 192, 192, 191, 191, 239, 191, 191, 239, 191, 191, 239, 191, 191, 239, 239, 192, 192, 239, 192, 144, 191, 190, 142, 191, 190, 190, 239, 189, 189, 239, 189, 189, 179, 179, 239, 131, 131, 239, 190, 190, 239, 239, 239, 188, 188, 191, 139, 188, 191, 191, 239, 239, 239, 239, 239, 239, 191, 143, 191, 191, 143, 191, 239, 239, 239, 239, 192, 192, 238, 192, 192, 191, 191, 238, 191, 191, 238, 238, 238, 238, 238, 238, 238, 238, 238, 191, 191, 238, 191, 191, 239, 239, 179, 179, 239, 180, 180, 240, 191, 192, 186, 137, 192, 186, 186, 240, 240, 192, 192, 240, 192, 192, 240, 240, 192, 192, 194, 146, 191, 194, 146, 191, 239, 191, 144, 191, 239, 191, 144, 191, 239, 136, 136, 239, 184, 136, 191, 179, 131, 145, 133, 179, 144, 144, 239, 191, 191, 190, 190, 190, 96, 86, 80, 95, 135, 78, 138, 188, 84, 79, 75, 23, 74, 82, 24, 125, 135, 77, 123, 122, 180, 121, 120, 236, 177, 120, 178, 232, 118, 62, 117, 73, 17, 55, 70, 68, 107, 58, 104, 150, 53, 98, 186, 136, 90, 130, 130, 49, 79, 82, 41, 75, 42, 40, 67, 55, 34, 58, 95, 70, 54, 66, 72, 56, 51, 57, 51, 46, 44, 38, 40, 38, 29, 42, 52, 49, 60, 74, 61, 43, 45, 28, 8, 7, 6, 5, 4, 4, 3, 3, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0
},

{
0, 0, 2, 3, 5, 6, 7, 9, 10, 12, 13, 14, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 29, 30, 31, 32, 33, 34, 36, 37, 38, 40, 41, 42, 43, 45, 46, 48, 49, 50, 52, 53, 54, 56, 57, 58, 60, 61, 63, 64, 65, 67, 68, 69, 71, 72, 74, 75, 76, 78, 79, 81, 82, 84, 85, 86, 88, 89, 91, 92, 94, 95, 97, 98, 100, 102, 103, 105, 106, 108, 109, 111, 112, 114, 115, 117, 118, 120, 121, 123, 124, 126, 127, 128, 130, 131, 133, 134, 136, 137, 139, 140, 141, 143, 144, 146, 147, 148, 150, 151, 152, 154, 155, 156, 158, 159, 160, 162, 163, 164, 166, 167, 168, 170, 171, 173, 174, 176, 177, 179, 180, 182, 183, 185, 187, 188, 190, 191, 193, 195, 196, 198, 199, 201, 203, 204, 206, 207, 209, 211, 212, 214, 215, 217, 219, 220, 222, 223, 225, 226, 228, 229, 231, 232, 234, 235, 237, 238, 239, 240, 241, 241, 242, 242, 242, 241, 241, 240, 239, 238, 237, 236, 235, 233, 232, 231, 229, 228, 227, 225, 224, 223, 221, 220, 219, 217, 216, 214, 213, 212, 210, 209, 208, 206, 205, 204, 202, 201, 200, 199, 197, 196, 195, 194, 192, 191, 190, 189, 188, 186, 185, 184, 183, 182, 180, 179, 178, 177, 175, 174, 173, 171, 170, 169, 168, 166, 165, 164, 162, 161, 160, 159, 157, 156, 155, 154, 152, 151, 150, 149, 148, 146, 145, 144, 143, 142, 140, 139, 138, 137, 135, 134, 133, 132, 130, 129, 128, 127, 125, 124, 123, 122, 121, 119, 118, 117, 116, 115, 113, 112, 111, 110, 109, 108, 106, 105, 104, 103, 102, 101, 100, 98, 97, 96, 95, 94, 93, 92, 91, 90, 88, 87, 86, 85, 84, 83, 82, 80, 79, 78, 77, 75, 74, 72, 71, 69, 68, 66, 65, 64, 62, 61, 59, 58, 57, 55, 54, 53, 52, 51, 49, 48, 47, 46, 45, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 27, 26, 25, 24, 23, 22, 21, 20, 19, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 7, 6, 5, 5, 4, 4, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
},


{
0, 0, 2, 3, 4, 5, 7, 8, 9, 11, 13, 14, 16, 18, 20, 22, 24, 26, 29, 31, 34, 37, 40, 43, 46, 50, 53, 57, 61, 65, 69, 74, 78, 83, 87, 92, 97, 102, 107, 112, 117, 122, 127, 132, 137, 143, 148, 153, 158, 163, 167, 172, 176, 181, 185, 189, 193, 197, 200, 204, 207, 210, 213, 215, 218, 220, 222, 224, 226, 227, 229, 230, 231, 232, 233, 233, 234, 234, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 235, 236, 236, 236, 236, 236, 237, 237, 237, 237, 237, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 239, 239, 239, 239, 239, 239, 239, 239, 239, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 239, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 239, 239, 239, 238, 237, 237, 236, 235, 233, 232, 230, 229, 227, 225, 222, 220, 217, 214, 210, 207, 203, 199, 195, 191, 186, 181, 176, 171, 166, 160, 155, 149, 143, 137, 131, 125, 119, 113, 107, 101, 95, 89, 83, 77, 72, 67, 62, 57, 52, 47, 43, 39, 36, 32, 29, 26, 23, 20, 18, 16, 14, 12, 11, 9, 8, 7, 6, 5, 4, 3, 3, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0
},

{
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 3, 3, 5, 6, 8, 10, 13, 16, 19, 22, 26, 30, 35, 40, 45, 50, 56, 62, 68, 74, 80, 86, 93, 99, 106, 112, 119, 126, 133, 140, 147, 154, 162, 170, 179, 188, 197, 206, 215, 223, 230, 236, 240, 243, 246, 247, 248, 249, 249, 248, 246, 244, 239, 230, 219, 206, 187, 167, 148, 127, 106, 90, 76, 63, 53, 47, 41, 36, 32, 28, 25, 22, 19, 17, 14, 12, 11, 9, 7, 6, 5, 4, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
},

{
0, 0, 2, 3, 71, 70, 7, 70, 115, 70, 115, 70, 16, 115, 18, 115, 115, 115, 22, 23, 154, 154, 154, 154, 154, 30, 31, 32, 33, 34, 36, 37, 186, 40, 186, 186, 43, 186, 186, 186, 186, 186, 52, 53, 54, 198, 198, 198, 198, 197, 195, 64, 65, 172, 171, 171, 71, 72, 74, 214, 214, 214, 214, 81, 214, 214, 214, 173, 173, 173, 173, 174, 174, 175, 176, 177, 179, 102, 103, 105, 106, 108, 204, 204, 112, 205, 205, 205, 207, 120, 121, 123, 124, 126, 127, 128, 130, 131, 133, 134, 136, 137, 139, 196, 194, 143, 193, 191, 147, 189, 150, 151, 152, 154, 155, 180, 180, 159, 179, 162, 163, 164, 179, 167, 168, 179, 171, 173, 174, 176, 179, 180, 180, 182, 183, 185, 187, 188, 190, 183, 184, 184, 196, 185, 199, 185, 203, 204, 206, 207, 209, 211, 212, 214, 188, 188, 188, 220, 222, 188, 225, 226, 188, 229, 231, 232, 234, 235, 237, 238, 239, 240, 195, 241, 187, 187, 195, 187, 241, 187, 187, 187, 192, 236, 235, 233, 232, 231, 229, 190, 227, 225, 224, 223, 189, 220, 219, 217, 189, 189, 213, 212, 187, 187, 208, 187, 205, 204, 187, 201, 200, 186, 197, 196, 181, 181, 181, 179, 181, 177, 181, 175, 174, 172, 171, 170, 157, 168, 167, 169, 166, 165, 163, 150, 162, 162, 160, 160, 159, 142, 158, 157, 156, 155, 154, 152, 151, 132, 150, 131, 147, 146, 144, 128, 143, 142, 140, 139, 138, 138, 136, 135, 134, 133, 131, 131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 122, 118, 78, 116, 115, 113, 77, 111, 110, 109, 108, 106, 76, 104, 103, 102, 101, 100, 75, 97, 96, 95, 94, 74, 92, 91, 90, 88, 73, 86, 72, 72, 83, 82, 80, 79, 78, 77, 75, 74, 72, 71, 69, 68, 66, 65, 64, 62, 61, 132, 58, 57, 132, 54, 53, 132, 51, 49, 48, 132, 46, 45, 43, 132, 41, 40, 133, 38, 37, 134, 35, 134, 33, 135, 31, 30, 135, 27, 135, 135, 24, 23, 22, 124, 124, 19, 124, 16, 124, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 5, 4, 124, 3, 3, 124, 2, 2, 122, 1, 226, 226, 225, 226, 227, 227, 227, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 1, 1, 1, 1, 118, 1, 118, 1, 118, 1, 2, 2, 118, 2, 2, 118, 2, 119, 119, 119, 119, 119, 2, 119, 119, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 96, 96, 0, 96, 0, 0, 96, 0, 96, 0, 0, 96, 0, 96, 96, 96, 96, 96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
}

};
