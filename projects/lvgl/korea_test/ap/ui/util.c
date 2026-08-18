#include <stdio.h>
#include <math.h>  // fabs() 함수를 위해 추가

// zeroAdd: 숫자를 두 자리 문자열로 변환 (sprintf 활용)
void zero_add(char* buf, int val) {
    sprintf(buf, "%02d", val);
}

// changeroff: 오프셋 인덱스를 실제 온도차(double)로 변환
double get_temp_offset(int index) {
    if (index == 0) return 0.0;
    if (index >= 1 && index <= 10) return (index * -0.5);
    if (index >= 11 && index <= 20) return (5.5 - (index * 0.5));
    return 0.0;
}

// changerod: 온도 편차 변환
double get_temp_deviation(int index) {
    double deviations[] = {0.2, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5};
    if (index >= 0 && index < 20) return deviations[index];
    return 0.2;
}

// 단위 변환 (섭씨 <-> 화씨)
double c_to_f_delta(double c) { return c * 1.8; }
double c_to_f_absolute(double c) { return (c * 1.8) + 32.0; }

/* ══════════════════════════════════════════════════════════════════
 * 유틸리티 함수
 * ══════════════════════════════════════════════════════════════════ */

/*
 * zeroAdd: 1자리 숫자 앞에 "0" 붙이기
 * in=NULL 이면 out을 그대로 유지 (자기자신 포맷용 snprintf 후 호출 시)
//  */
// void zero_add(const char *in, char *out, size_t out_size)
// {
//     if (!in) return;
//     if (strlen(in) == 1)
//         snprintf(out, out_size, "0%s", in);
//     else
//         snprintf(out, out_size, "%s", in);
// }

/* changeroff: 인덱스 → 오프셋 값 변환 */
double changer_off(double off)
{
    static const double tbl[] = {
        0, -0.5, -1.0, -1.5, -2.0, -2.5, -3.0, -3.5, -4.0, -4.5, -5.0,
        5.0, 4.5, 4.0, 3.5, 3.0, 2.5, 2.0, 1.5, 1.0, 0.5
    };
    int idx = (int)off;
    if (idx >= 0 && idx <= 20) return tbl[idx];
    return 0;
}

/* reversechangeroff: 오프셋 값 → 인덱스 역변환 */
double reverse_changer_off(double off)
{
    static const struct { double v; double i; } tbl[] = {
        {0,0},{-0.5,1},{-1.0,2},{-1.5,3},{-2.0,4},{-2.5,5},{-3.0,6},
        {-3.5,7},{-4.0,8},{-4.5,9},{-5.0,10},{5.0,11},{4.5,12},
        {4.0,13},{3.5,14},{3.0,15},{2.5,16},{2.0,17},{1.5,18},{1.0,19},{0.5,20}
    };
    for (int i = 0; i < 21; i++)
        if (fabs(off - tbl[i].v) < 0.001) return tbl[i].i;
    return 0;
}

/* changerod: 인덱스 → od 값 */
double changer_od(double od)
{
    static const double tbl[] = {
        0.2, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5,
        5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5
    };
    int idx = (int)od;
    if (idx >= 0 && idx <= 19) return tbl[idx];
    return od;
}

/* reversechangerod: od 값 → 인덱스 역변환 */
double reverse_changer_od(double od)
{
    static const double tbl[] = {
        0.2, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5,
        5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5
    };
    for (int i = 0; i < 20; i++)
        if (fabs(od - tbl[i]) < 0.001) return (double)i;
    return od;
}

/* 섭씨 → 화씨 배율 */
double degree_change(double ch)       { return ch * 1.8; }
double degree_basic_change(double ch) { return ch * 1.8 + 32.0; }
double reverse_degree_change(double ch)       { return ch / 1.8; }
double reverse_degree_basic_change(double ch) { return (ch - 32.0) / 1.8; }

/* truefalse: "1" → "ON", 그 외 → "OFF" */
const char *true_false_str(const char *val)
{
    return (val && val[0] == '1') ? "ON" : "OFF";
}
