#pragma once
float skyboxVertexCount = 36;

float skyboxVertices[] = {

		-1.0f,  1.0f, -1.0f, /* back */
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f, /* left */
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f,  1.0f, -1.0f, /* right */
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,

		 1.0f,  1.0f,  1.0f, /* front */
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f, /* top */
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f, /* bottom - need to check */
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
};

/*
-1.0f, 1.0f, -1.0f,  back
-1.0f, -1.0f, -1.0f,
1.0f, -1.0f, -1.0f,
1.0f, -1.0f, -1.0f,
1.0f, 1.0f, -1.0f,
-1.0f, 1.0f, -1.0f,

-1.0f, -1.0f, 1.0f, left
-1.0f, -1.0f, -1.0f,
-1.0f, 1.0f, -1.0f,
-1.0f, 1.0f, -1.0f,
-1.0f, 1.0f, 1.0f,
-1.0f, -1.0f, 1.0f,

1.0f, -1.0f, -1.0f,  right
1.0f, -1.0f, 1.0f,
1.0f, 1.0f, 1.0f,
1.0f, 1.0f, 1.0f,
1.0f, 1.0f, -1.0f,
1.0f, -1.0f, -1.0f

- 1.0f, -1.0f, 1.0f,  front
-1.0f, 1.0f, 1.0f,
1.0f, 1.0f, 1.0f,
1.0f, 1.0f, 1.0f,
1.0f, -1.0f, 1.0f,
-1.0f, -1.0f, 1.0f,

-1.0f, 1.0f, -1.0f,  top
1.0f, 1.0f, -1.0f,
1.0f, 1.0f, 1.0f,
1.0f, 1.0f, 1.0f,
-1.0f, 1.0f, 1.0f,
-1.0f, 1.0f, -1.0f,

-1.0f, -1.0f, -1.0f,  bottom - need to check
-1.0f, -1.0f, 1.0f,
1.0f, -1.0f, -1.0f,
1.0f, -1.0f, -1.0f,
-1.0f, -1.0f, 1.0f,
1.0f, -1.0f, 1.0f*/