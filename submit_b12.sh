#!/usr/bin/env bash
set -euo pipefail

# ===========================
# CONFIG (static fields)
# ===========================

NAME="Hicham Elmefeddel"
EMAIL="elmefeddel.hicham@outlook.com"
RESUME_LINK="https://hel-mefe.me/assets/Hicham_Elmefeddel_CV-DIxH2CJd.pdf"
REPOSITORY_LINK="https://github.com/hel-mefe/Nginx-similar-webserver-42"

SIGNING_SECRET="hello-there-from-b12"
ENDPOINT="https://b12.io/apply/submission"

# ===========================
# REQUIRED ENV VAR
# ===========================

if [[ -z "${ACTION_RUN_LINK:-}" ]]; then
  echo "❌ ERROR: ACTION_RUN_LINK is not set"
  echo "👉 Example:"
  echo "   ACTION_RUN_LINK=https://github.com/.../actions/runs/ID ./submit_b12.sh"
  exit 1
fi

# ===========================
# TIMESTAMP (ISO 8601 UTC)
# ===========================

TIMESTAMP=$(date -u +"%Y-%m-%dT%H:%M:%S.%3NZ")

# ===========================
# CANONICAL JSON PAYLOAD
# (sorted keys, compact)
# ===========================

PAYLOAD=$(printf '{"action_run_link":"%s","email":"%s","name":"%s","repository_link":"%s","resume_link":"%s","timestamp":"%s"}' \
"$ACTION_RUN_LINK" \
"$EMAIL" \
"$NAME" \
"$REPOSITORY_LINK" \
"$RESUME_LINK" \
"$TIMESTAMP")

# ===========================
# HMAC-SHA256 SIGNATURE
# ===========================

SIGNATURE=$(printf '%s' "$PAYLOAD" | \
  openssl dgst -sha256 -hmac "$SIGNING_SECRET" | \
  sed 's/^.* //')

# ===========================
# SUBMIT REQUEST
# ===========================

echo "🚀 Submitting to B12..."

RESPONSE=$(curl -sS -X POST "$ENDPOINT" \
  -H "Content-Type: application/json" \
  -H "X-Signature-256: sha256=$SIGNATURE" \
  --data "$PAYLOAD")

echo "📦 Response:"
echo "$RESPONSE"

# ===========================
# EXTRACT RECEIPT
# ===========================

RECEIPT=$(echo "$RESPONSE" | sed -n 's/.*"receipt":"\([^"]*\)".*/\1/p')

if [[ -z "$RECEIPT" ]]; then
  echo "❌ Submission failed (receipt missing)"
  exit 1
fi

echo
echo "✅ SUCCESS"
echo "🎟️  B12 RECEIPT: $RECEIPT"

