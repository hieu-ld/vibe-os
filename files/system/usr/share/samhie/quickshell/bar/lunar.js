/*
 * Đổi ngày dương sang ngày âm lịch VIỆT NAM.
 *
 * Vì sao phải tự tính, không dùng plugin sẵn của Plasma: ISO có
 * plasmacalendarplugins/alternatecalendar.so và nó biết hệ "Chinese", nhưng đó
 * là âm lịch tính theo múi giờ Trung Quốc (UTC+8). Âm lịch Việt Nam tính theo
 * UTC+7. Hai lịch trùng nhau gần hết, chỉ lệch vào những năm mà thời điểm sóc
 * (new moon) rơi đúng vào khoảng 23:00-24:00 giờ Bắc Kinh — khi đó ngày mùng 1
 * ở Việt Nam sớm hơn một ngày, và cả tháng đó lệch theo. Năm 1968 và 1985 là
 * hai lần Tết hai nước lệch nhau vì đúng lý do này. Plugin có ô "dateOffset"
 * nhưng đó là cộng thêm một số ngày CỐ ĐỊNH, mà chênh lệch ở đây không cố định
 * — cộng bù cho vài ngày lệch thì làm sai hơn 350 ngày còn lại.
 *
 * Thuật toán: bản của Hồ Ngọc Đức (dựa trên Astronomical Algorithms của Jean
 * Meeus), là bản được dùng rộng rãi nhất cho âm lịch Việt Nam. Tham số timeZone
 * truyền vào là 7.
 *
 * Có bộ kiểm tra chạy được ngoài Plasma: `node tools/test-lunar.js`.
 */

.pragma library

var TIMEZONE_VN = 7;

function INT( d ) { return Math.floor( d ); }

// Số ngày Julius của một ngày dương lịch.
function jdFromDate( dd, mm, yy ) {
    var a = INT( ( 14 - mm ) / 12 );
    var y = yy + 4800 - a;
    var m = mm + 12 * a - 3;
    var jd = dd + INT( ( 153 * m + 2 ) / 5 ) + 365 * y
           + INT( y / 4 ) - INT( y / 100 ) + INT( y / 400 ) - 32045;
    if ( jd < 2299161 ) {
        // Trước 15/10/1582 thì là lịch Julius, không phải Gregory.
        jd = dd + INT( ( 153 * m + 2 ) / 5 ) + 365 * y + INT( y / 4 ) - 32083;
    }
    return jd;
}

function jdToDate( jd ) {
    var a, b, c, d, e, m;
    if ( jd > 2299160 ) {
        a = jd + 32044;
        b = INT( ( 4 * a + 3 ) / 146097 );
        c = a - INT( ( b * 146097 ) / 4 );
    } else {
        b = 0;
        c = jd + 32082;
    }
    d = INT( ( 4 * c + 3 ) / 1461 );
    e = c - INT( ( 1461 * d ) / 4 );
    m = INT( ( 5 * e + 2 ) / 153 );
    return [ e - INT( ( 153 * m + 2 ) / 5 ) + 1,      // ngày
             m + 3 - 12 * INT( m / 10 ),              // tháng
             b * 100 + d - 4800 + INT( m / 10 ) ];    // năm
}

// Thời điểm sóc thứ k tính từ điểm sóc ngày 1/1/1900, trả về ngày Julius lẻ.
function newMoon( k ) {
    var T  = k / 1236.85;          // thế kỷ Julius tính từ 1900-01-00.5
    var T2 = T * T;
    var T3 = T2 * T;
    var dr = Math.PI / 180;

    var Jd1 = 2415020.75933 + 29.53058868 * k + 0.0001178 * T2 - 0.000000155 * T3;
    Jd1 = Jd1 + 0.00033 * Math.sin( ( 166.56 + 132.87 * T - 0.009173 * T2 ) * dr );

    var M   = 359.2242   + 29.10535608  * k - 0.0000333 * T2 - 0.00000347 * T3;
    var Mpr = 306.0253   + 385.81691806 * k + 0.0107306 * T2 + 0.00001236 * T3;
    var F   =  21.2964   + 390.67050646 * k - 0.0016528 * T2 - 0.00000239 * T3;

    var C1 = ( 0.1734 - 0.000393 * T ) * Math.sin( M * dr ) + 0.0021 * Math.sin( 2 * dr * M );
    C1 = C1 - 0.4068 * Math.sin( Mpr * dr ) + 0.0161 * Math.sin( dr * 2 * Mpr );
    C1 = C1 - 0.0004 * Math.sin( dr * 3 * Mpr );
    C1 = C1 + 0.0104 * Math.sin( dr * 2 * F ) - 0.0051 * Math.sin( dr * ( M + Mpr ) );
    C1 = C1 - 0.0074 * Math.sin( dr * ( M - Mpr ) ) + 0.0004 * Math.sin( dr * ( 2 * F + M ) );
    C1 = C1 - 0.0004 * Math.sin( dr * ( 2 * F - M ) ) - 0.0006 * Math.sin( dr * ( 2 * F + Mpr ) );
    C1 = C1 + 0.0010 * Math.sin( dr * ( 2 * F - Mpr ) ) + 0.0005 * Math.sin( dr * ( 2 * Mpr + M ) );

    var deltat;
    if ( T < -11 ) {
        deltat = 0.001 + 0.000839 * T + 0.0002261 * T2 - 0.00000845 * T3 - 0.000000081 * T * T3;
    } else {
        deltat = -0.000278 + 0.000265 * T + 0.000262 * T2;
    }
    return Jd1 + C1 - deltat;
}

// Kinh độ mặt trời, radian.
function sunLongitude( jdn ) {
    var T  = ( jdn - 2451545.0 ) / 36525;
    var T2 = T * T;
    var dr = Math.PI / 180;

    var M  = 357.52910 + 35999.05030 * T - 0.0001559 * T2 - 0.00000048 * T * T2;
    var L0 = 280.46645 + 36000.76983 * T + 0.0003032 * T2;

    var DL = ( 1.914600 - 0.004817 * T - 0.000014 * T2 ) * Math.sin( dr * M );
    DL = DL + ( 0.019993 - 0.000101 * T ) * Math.sin( dr * 2 * M )
            + 0.000290 * Math.sin( dr * 3 * M );

    var L = ( L0 + DL ) * dr;
    return L - Math.PI * 2 * INT( L / ( Math.PI * 2 ) );
}

// Kinh độ mặt trời quy về 12 cung (0..11) lúc 0h địa phương.
function getSunLongitude( dayNumber, timeZone ) {
    return INT( sunLongitude( dayNumber - 0.5 - timeZone / 24 ) / Math.PI * 6 );
}

// Ngày (số nguyên, giờ địa phương) chứa điểm sóc thứ k.
function getNewMoonDay( k, timeZone ) {
    return INT( newMoon( k ) + 0.5 + timeZone / 24 );
}

// Ngày bắt đầu tháng 11 âm lịch của năm dương yy — mốc để đánh số các tháng.
function getLunarMonth11( yy, timeZone ) {
    var off = jdFromDate( 31, 12, yy ) - 2415021;
    var k = INT( off / 29.530588853 );
    var nm = getNewMoonDay( k, timeZone );
    if ( getSunLongitude( nm, timeZone ) >= 9 ) {
        nm = getNewMoonDay( k - 1, timeZone );
    }
    return nm;
}

// Tháng nhuận nằm ở vị trí thứ mấy sau tháng 11 — tháng đầu tiên không chứa
// trung khí.
function getLeapMonthOffset( a11, timeZone ) {
    var k = INT( ( a11 - 2415021.076998695 ) / 29.530588853 + 0.5 );
    var last = 0;
    var i = 1;
    var arc = getSunLongitude( getNewMoonDay( k + i, timeZone ), timeZone );
    do {
        last = arc;
        i++;
        arc = getSunLongitude( getNewMoonDay( k + i, timeZone ), timeZone );
    } while ( arc !== last && i < 14 );
    return i - 1;
}

/*
 * Đổi ngày dương sang âm. Trả về [ ngày, tháng, năm, nhuận ] với nhuận là 0/1.
 * timeZone bỏ trống thì lấy 7 (Việt Nam).
 */
function solar2Lunar( dd, mm, yy, timeZone ) {
    if ( timeZone === undefined ) { timeZone = TIMEZONE_VN; }

    var dayNumber = jdFromDate( dd, mm, yy );
    var k = INT( ( dayNumber - 2415021.076998695 ) / 29.530588853 );

    var monthStart = getNewMoonDay( k + 1, timeZone );
    if ( monthStart > dayNumber ) {
        monthStart = getNewMoonDay( k, timeZone );
    }

    var a11 = getLunarMonth11( yy, timeZone );
    var b11 = a11;
    var lunarYear;
    if ( a11 >= monthStart ) {
        lunarYear = yy;
        a11 = getLunarMonth11( yy - 1, timeZone );
    } else {
        lunarYear = yy + 1;
        b11 = getLunarMonth11( yy + 1, timeZone );
    }

    var lunarDay = dayNumber - monthStart + 1;
    var diff = INT( ( monthStart - a11 ) / 29 );
    var lunarLeap = 0;
    var lunarMonth = diff + 11;

    if ( b11 - a11 > 365 ) {
        var leapMonthDiff = getLeapMonthOffset( a11, timeZone );
        if ( diff >= leapMonthDiff ) {
            lunarMonth = diff + 10;
            if ( diff === leapMonthDiff ) { lunarLeap = 1; }
        }
    }
    if ( lunarMonth > 12 ) { lunarMonth = lunarMonth - 12; }
    if ( lunarMonth >= 11 && diff < 4 ) { lunarYear -= 1; }

    return [ lunarDay, lunarMonth, lunarYear, lunarLeap ];
}

var CAN = [ "Giáp", "Ất", "Bính", "Đinh", "Mậu", "Kỷ", "Canh", "Tân", "Nhâm", "Quý" ];
var CHI = [ "Tý", "Sửu", "Dần", "Mão", "Thìn", "Tỵ", "Ngọ", "Mùi", "Thân", "Dậu", "Tuất", "Hợi" ];

// Tên can chi của năm âm, ví dụ 2026 -> "Bính Ngọ".
function canChiYear( lunarYear ) {
    return CAN[ ( lunarYear + 6 ) % 10 ] + " " + CHI[ ( lunarYear + 8 ) % 12 ];
}
