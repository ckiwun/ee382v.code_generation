int A[10];
int g;

int extra();

int small(int s)
{
	int x = s, y = s, a = s, b = s, c = s, d = s;
	x = c * d;
	y = x / 2;
	if (s > 0) {
		a = x + y;
		x = a + b;
	}
	return x;
}