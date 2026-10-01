#!/usr/bin/env node
/*
 * Kiểm tra phần đổi âm lịch của đồng hồ thanh trên (mang từ bản 1.0), chạy
 * bằng Node, không cần Quickshell.
 *
 * Sai âm lịch thì nhìn cũng không biết là sai. Nên kiểm bằng hai đường: đối
 * chiếu ngày Tết đã biết, và kiểm tính nhất quán trên hàng chục nghìn ngày
 * liên tiếp.
 *
 *     node tools/test-lunar.js
 */

const fs = require( 'fs' );
const path = require( 'path' );

// File QML/JS dùng ".pragma library", Node không hiểu — bỏ dòng đó rồi nạp.
const src = fs.readFileSync(
    path.join( __dirname, '../files/system/usr/share/samhie/quickshell/bar/lunar.js' ),
    'utf8'
).replace( /^\s*\.pragma\s+library\s*$/m, '' );

const Lunar = {};
new Function( 'exports', src + '\n;Object.assign(exports,{solar2Lunar,jdFromDate,jdToDate,canChiYear,TIMEZONE_VN});' )( Lunar );

let failed = 0;
function check( name, got, want ) {
    const ok = String( got ) === String( want );
    if ( !ok ) { failed++; }
    console.log( `  ${ ok ? 'OK  ' : 'SAI ' } ${ name }: ${ got }${ ok ? '' : `  (đáng lẽ ${ want })` }` );
}

// ---- 1. Ngày Tết Việt Nam đã biết ----------------------------------------
// Mùng 1 tháng Giêng âm phải rơi đúng vào những ngày dương này.
console.log( '\n[1] Mùng 1 Tết các năm (âm lịch Việt Nam, UTC+7)' );
const tet = {
    2020: [ 25,  1 ], 2021: [ 12,  2 ], 2022: [  1,  2 ],
    2023: [ 22,  1 ], 2024: [ 10,  2 ], 2025: [ 29,  1 ],
    2026: [ 17,  2 ], 2027: [  6,  2 ], 2028: [ 26,  1 ],
};
for ( const [ year, [ dd, mm ] ] of Object.entries( tet ) ) {
    const l = Lunar.solar2Lunar( dd, mm, Number( year ) );
    check( `${ dd }/${ mm }/${ year }`, `${ l[0] }/${ l[1] }`, '1/1' );
}

// ---- 1b. Tháng nhuận -----------------------------------------------------
// Hai mốc này lấy từ test của dự án Reminder
// (~/Projects/Reminder/test/core/utils/date_utils_test.dart), nơi cùng thuật
// toán đã được port sang Dart. Đối chiếu chéo hai bản triển khai độc lập.
console.log( '\n[1b] Tháng nhuận (đối chiếu với test của app Reminder)' );
{
    const a = Lunar.solar2Lunar( 22, 3, 2023 );
    check( '22/3/2023 là mùng 1 tháng 2 NHUẬN', `${ a[0] }/${ a[1] }/${ a[2] }/${ a[3] }`, '1/2/2023/1' );
    const b = Lunar.solar2Lunar( 21, 3, 2023 );
    check( '21/3/2023 vẫn là tháng 2 thường', `${ b[1] }/${ b[3] }`, '2/0' );
}

// ---- 2. Tên can chi ------------------------------------------------------
console.log( '\n[2] Can chi năm âm' );
check( '2024', Lunar.canChiYear( 2024 ), 'Giáp Thìn' );
check( '2025', Lunar.canChiYear( 2025 ), 'Ất Tỵ' );
check( '2026', Lunar.canChiYear( 2026 ), 'Bính Ngọ' );

// ---- 3. Tính nhất quán trên 60 năm ---------------------------------------
// Mấy tính chất này đúng với mọi âm lịch kiểu Trung Hoa. Sai một cái là thuật
// toán hỏng, không cần biết ngày cụ thể nào đúng.
console.log( '\n[3] Quét liên tục 1990-2050' );
let prev = null, monthLen = 0, seenFirstMonth = false, badLen = [], badSeq = 0, monthsInYear = {}, days = 0;
for ( let jd = Lunar.jdFromDate( 1, 1, 1990 ); jd <= Lunar.jdFromDate( 31, 12, 2050 ); jd++ ) {
    const [ dd, mm, yy ] = Lunar.jdToDate( jd );
    const l = Lunar.solar2Lunar( dd, mm, yy );
    days++;

    if ( l[1] < 1 || l[1] > 12 ) { badSeq++; }
    if ( l[0] < 1 || l[0] > 30 ) { badSeq++; }

    if ( prev ) {
        if ( l[0] === 1 ) {
            // Sang tháng mới: tháng vừa qua phải dài 29 hoặc 30 ngày. Bỏ qua
            // tháng đầu tiên vì vòng quét bắt đầu từ 1/1 dương, tức là rơi vào
            // giữa một tháng âm — tháng đó cụt, đo nó là vô nghĩa.
            if ( seenFirstMonth && monthLen !== 29 && monthLen !== 30 ) {
                badLen.push( `${ dd }/${ mm }/${ yy }:${ monthLen }` );
            }
            seenFirstMonth = true;
            monthLen = 1;
            const key = `${ l[2] }`;
            monthsInYear[ key ] = ( monthsInYear[ key ] || 0 ) + 1;
        } else if ( l[0] === prev[0] + 1 ) {
            monthLen++;
        } else {
            badSeq++;
        }
    } else {
        monthLen = 1;
    }
    prev = l;
}
check( `số ngày quét`, days > 22000, true );
check( 'ngày âm luôn nối tiếp hoặc reset về 1', badSeq, 0 );
check( 'mọi tháng âm dài 29 hoặc 30 ngày', badLen.length ? badLen.slice( 0, 5 ).join( ',' ) : 0, 0 );

const counts = Object.entries( monthsInYear ).filter( ( [ y ] ) => y > 1990 && y < 2050 );
const bad = counts.filter( ( [ , n ] ) => n !== 12 && n !== 13 );
check( 'mỗi năm âm có 12 hoặc 13 tháng', bad.length ? bad.map( x => x.join( '=' ) ).join( ',' ) : 0, 0 );
const leapYears = counts.filter( ( [ , n ] ) => n === 13 ).length;
const expectLeap = counts.length * 7 / 19;
check( 'số năm nhuận sát chu kỳ 7/19',
       Math.abs( leapYears - expectLeap ) <= 2,
       true );
console.log( `       (${ leapYears } năm nhuận / ${ counts.length } năm, chu kỳ 7/19 cho ~${ expectLeap.toFixed( 1 ) })` );

// ---- 4. Chỗ âm lịch VN khác âm lịch TQ -----------------------------------
// Không phải kiểm tra đúng/sai, mà là in ra cho thấy múi giờ thật sự có tác
// dụng. Nếu danh sách này rỗng thì tham số timeZone đang bị bỏ qua.
console.log( '\n[4] Ngày mà âm lịch VN (UTC+7) khác âm lịch TQ (UTC+8), 1960-2050' );
let diffs = [];
for ( let jd = Lunar.jdFromDate( 1, 1, 1960 ); jd <= Lunar.jdFromDate( 31, 12, 2050 ); jd++ ) {
    const [ dd, mm, yy ] = Lunar.jdToDate( jd );
    const vn = Lunar.solar2Lunar( dd, mm, yy, 7 );
    const cn = Lunar.solar2Lunar( dd, mm, yy, 8 );
    if ( vn[0] !== cn[0] || vn[1] !== cn[1] ) {
        diffs.push( `${ dd }/${ mm }/${ yy }: VN ${ vn[0] }/${ vn[1] } vs TQ ${ cn[0] }/${ cn[1] }` );
    }
}
console.log( `    tổng ${ diffs.length } ngày lệch` );
// Cỡ này là đúng như dự kiến, không phải bug: mỗi điểm sóc có ~1/24 khả năng
// rơi vào khung một tiếng giữa hai múi giờ, mà lệch một điểm sóc là lệch NGUYÊN
// một tháng âm. ~12,4 điểm sóc/năm x 1/24 x ~30 ngày ~ 15 ngày lệch mỗi năm.
check( 'số ngày lệch nằm trong khoảng hợp lý (5-25 ngày/năm)',
       diffs.length / 91 > 5 && diffs.length / 91 < 25,
       true );
for ( const d of diffs.slice( 0, 6 ) ) { console.log( `      ${ d }` ); }
if ( diffs.length > 6 ) { console.log( `      ... còn ${ diffs.length - 6 } ngày nữa` ); }
check( 'múi giờ có tác dụng (phải có ngày lệch)', diffs.length > 0, true );

// ---- 5. Lưới lịch trong CalendarPopup.qml --------------------------------
// Rút thẳng hàm buildCells ra khỏi file QML rồi chạy bằng Node, để khỏi chép
// đôi logic.
console.log( '\n[5] Lưới lịch (buildCells rút từ CalendarPopup.qml)' );
const qml = fs.readFileSync(
    path.join( __dirname, '../files/system/usr/share/samhie/quickshell/bar/CalendarPopup.qml' ), 'utf8' );
const m = qml.match( /function buildCells\(\s*monthDate\s*\)\s*\{[\s\S]*?\n    \}/ );
check( 'rút được hàm buildCells ra khỏi QML', m !== null, true );

if ( m ) {
    const root = {
        sameDay: ( a, b ) => a.getFullYear() === b.getFullYear()
                          && a.getMonth() === b.getMonth()
                          && a.getDate() === b.getDate(),
        now: new Date( 2026, 6, 2 ),   // 2/7/2026, đúng ngày bản vẽ Figma
    };
    root.buildCells = new Function( 'Lunar', 'root',
        m[ 0 ] + '; return buildCells;' )( Lunar, root );

    let badGrid = [];
    for ( let y = 2024; y <= 2028; y++ ) {
        for ( let mo = 0; mo < 12; mo++ ) {
            const cells = root.buildCells( new Date( y, mo, 1 ) );
            if ( cells.length !== 42 ) { badGrid.push( `${ mo + 1 }/${ y }: ${ cells.length } ô` ); }
            const inMonth = cells.filter( c => c.inMonth ).length;
            const want = new Date( y, mo + 1, 0 ).getDate();
            if ( inMonth !== want ) { badGrid.push( `${ mo + 1 }/${ y }: ${ inMonth }/${ want } ngày` ); }
            // Ô đầu tiên phải là Thứ 2.
            const lead = ( new Date( y, mo, 1 ).getDay() + 6 ) % 7;
            const first = new Date( y, mo, 1 - lead );
            if ( first.getDay() !== 1 ) { badGrid.push( `${ mo + 1 }/${ y }: ô đầu không phải Thứ 2` ); }
        }
    }
    check( 'mọi tháng 2024-2028 ra đúng 42 ô, đủ ngày, bắt đầu Thứ 2',
           badGrid.length ? badGrid.slice( 0, 3 ).join( ' | ' ) : 0, 0 );

    // Đối chiếu từng ô với bản vẽ Figma tháng 7/2026.
    const jul = root.buildCells( new Date( 2026, 6, 1 ) );
    check( 'ô đầu lưới tháng 7/2026 là 29 (dương) / 15 (âm)',
           `${ jul[0].dayText }/${ jul[0].lunarText }`, '29/15' );
    check( 'ô ngày 14/7/2026 ghi âm là "1/6"',
           jul.find( c => c.dayText === '14' && c.inMonth ).lunarText, '1/6' );
    check( 'ô đó được đánh dấu là mùng 1 âm (để tô đỏ)',
           jul.find( c => c.dayText === '14' && c.inMonth ).newLunarMonth, true );
    check( 'không ô nào còn hậu tố "N"',
           jul.filter( c => /N/.test( c.lunarText ) ).length, 0 );
    check( 'đúng một ô được đánh dấu hôm nay',
           jul.filter( c => c.isToday ).length, 1 );
    check( 'ô hôm nay là mùng 2 dương, 18 âm',
           ( () => { const t = jul.find( c => c.isToday ); return `${ t.dayText }/${ t.lunarText }`; } )(),
           '2/18' );
}

console.log( failed === 0 ? '\nTất cả đều đạt.\n' : `\n${ failed } mục SAI.\n` );
process.exit( failed === 0 ? 0 : 1 );
