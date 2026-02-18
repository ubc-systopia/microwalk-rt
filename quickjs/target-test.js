import { exported_function, other_exported_function } from "./test.js";

function other_function(arg) {
    return arg && arg % 2;
}

export function test(arg) {
    let res = 0;

    function nested_function(arg) {
        return arg && arg % 3;
    }

    const closure = (arg) => arg && arg % 6;

    res += other_function(arg);
    res += nested_function(arg);
    res += exported_function(arg);
    res += other_exported_function(arg);
    res += closure(arg);
    return res;
}
