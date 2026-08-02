# Block / warn when a git commit command embeds a Cursor co-author trailer.
$ErrorActionPreference = "Stop"
$raw = [Console]::In.ReadToEnd()
try {
    $payload = $raw | ConvertFrom-Json
} catch {
    Write-Output '{"permission":"allow"}'
    exit 0
}

$cmd = ""
if ($payload.command) { $cmd = [string]$payload.command }
elseif ($payload.tool_input.command) { $cmd = [string]$payload.tool_input.command }

if ($cmd -match 'Co-authored-by:\s*.*Cursor' -or $cmd -match 'cursoragent@cursor\.com') {
    $out = @{
        permission = "deny"
        userMessage = "Commit blocked: remove Co-authored-by Cursor from the git commit command."
        agentMessage = "Do not include Co-authored-by: Cursor in commits. Rewrite the commit message without that trailer and retry."
    } | ConvertTo-Json -Compress
    Write-Output $out
    exit 0
}

Write-Output '{"permission":"allow"}'
exit 0
