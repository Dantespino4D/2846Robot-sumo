import re

files_estrategia = [
    '/home/dante/Documentos/robot/esp32-s3/components/core/Estrategia1.cpp',
    '/home/dante/Documentos/robot/esp32-s3/components/core/Estrategia2.cpp'
]

for f in files_estrategia:
    with open(f, 'r') as file:
        content = file.read()
    
    # We want to remove cases 0, 1, 2, 3 completely
    content = re.sub(r'\t+case 0:.*?\n\t+case 1:.*?\n\t+case 2:.*?\n\t+case 3:.*?break;\n', '', content, flags=re.DOTALL)
    
    # Now we decrement every remaining case N: by 4
    def dec_case(match):
        num = int(match.group(2))
        return f'{match.group(1)}{num - 4}:'
    content = re.sub(r'(\t*case\s+)(\d+):', dec_case, content)
    
    with open(f, 'w') as file:
        file.write(content)

f_est_est = '/home/dante/Documentos/robot/esp32-s3/components/core/EstrategiaEstandar.cpp'
with open(f_est_est, 'r') as file:
    content = file.read()

# Remove the color sensor block:
to_remove = r'''\t//se verifica si se detecto algun sensor de color el que sea
\tif\(noti & MASK_COLOR\)\{
\t\t// si detecta el limite por la direccion A
\t\tif \(noti & MASK_LIM_A\) \{
\t\t\tctx->modo = 0;
\t\t\}
\t\t// si detecta el limite por la direccion B
\t\telse if \(noti & MASK_LIM_B\) \{
\t \t\t ctx->modo = 1;
\t\t\}
\t//se verifica si hay alguna memoria del sensor de color
\t\}else if\(ctx->memo_C != 0\) \{
\t\t// si deja de detectar el limite por sc 1
\t\tif \(ctx->memo_C == 1\) \{
\t\t\tctx->modo = 2;
\t\t\}
\t\t// si deja de detectar el limite por sc 2
\t\telse if \(ctx->memo_C == 2\) \{
\t\t\tctx->modo = 3;
\t\t\}
\t\}else if\(ctx->stall\) \{'''
replacement = r'\tif(ctx->stall) {'
content = re.sub(to_remove, replacement, content)

def dec_modo(match):
    num = int(match.group(2))
    return f'{match.group(1)}{num - 4}'
content = re.sub(r'(ctx->modo\s*=\s*)(\d+)', dec_modo, content)

def dec_ternary(match):
    n1 = int(match.group(2))
    n2 = int(match.group(4))
    return f'{match.group(1)}{n1 - 4} : {n2 - 4}'
content = re.sub(r'(\? )(\d+)( : )(\d+)', dec_ternary, content)

with open(f_est_est, 'w') as file:
    file.write(content)

print("Done with strategies.")
